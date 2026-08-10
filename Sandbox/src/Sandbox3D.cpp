#include "Sandbox3D.h"
// ----------------------------------------

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <functional>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <random>
#include <unordered_map>
#include <vector>

#include "../../Achengine/vendor/Glad/include/glad/glad.h"
#include "../../Achengine/vendor/GLFW/include/GLFW/glfw3.h"

static glm::vec3 ToTransform(glm::vec3 vec)
{
	return glm::vec3(vec.y, vec.z, vec.x);
}

static bool FileExists(const std::string& path)
{
	std::ifstream stream(path);
	return stream.good();
}

static void SetPlayCursorCaptured(bool captured)
{
	Achengine::Application* app = Achengine::Application::Get();
	if (!app)
	{
		return;
	}

	GLFWwindow* window = static_cast<GLFWwindow*>(app->GetWindow().GetNativeWindow());
	if (!window)
	{
		return;
	}

	glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

static std::string GetDirectoryFromPath(const std::string& filePath)
{
	const size_t slash = filePath.find_last_of("/\\");
	if (slash == std::string::npos)
	{
		return ".";
	}

	return filePath.substr(0, slash);
}

static bool WriteTextFileAtomically(const std::string& filePath, const std::string& contents, std::string& outError)
{
	if (filePath.empty())
	{
		outError = "Target path is empty";
		return false;
	}

	const std::string tempPath = filePath + ".tmp";
	{
		std::ofstream out(tempPath, std::ios::out | std::ios::trunc);
		if (!out.is_open())
		{
			outError = Achengine::format("Failed to open temp file for write: %s", tempPath.c_str());
			return false;
		}

		out << contents;
		if (!out.good())
		{
			outError = Achengine::format("Failed while writing temp file: %s", tempPath.c_str());
			out.close();
			std::remove(tempPath.c_str());
			return false;
		}

		out.flush();
		if (!out.good())
		{
			outError = Achengine::format("Failed while flushing temp file: %s", tempPath.c_str());
			out.close();
			std::remove(tempPath.c_str());
			return false;
		}
	}

	std::remove(filePath.c_str());
	if (std::rename(tempPath.c_str(), filePath.c_str()) != 0)
	{
		outError = Achengine::format("Failed to replace map file (%s)", std::strerror(errno));
		std::remove(tempPath.c_str());
		return false;
	}

	return true;
}

static bool IsModelFile(const std::string& fileName)
{
	const size_t dot = fileName.find_last_of('.');
	if (dot == std::string::npos)
	{
		return false;
	}

	std::string ext = fileName.substr(dot + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return ext == "fbx" || ext == "obj" || ext == "gltf" || ext == "glb" || ext == "dae";
}

static bool IsJsonFile(const std::string& fileName)
{
	const size_t dot = fileName.find_last_of('.');
	if (dot == std::string::npos)
	{
		return false;
	}

	std::string ext = fileName.substr(dot + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return ext == "json";
}

static bool IsTextureFile(const std::string& fileName)
{
	const size_t dot = fileName.find_last_of('.');
	if (dot == std::string::npos)
	{
		return false;
	}

	std::string ext = fileName.substr(dot + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "tga" || ext == "bmp" || ext == "dds" || ext == "hdr";
}

static bool IsGameAssetFile(const std::string& fileName)
{
	return IsModelFile(fileName) || IsTextureFile(fileName) || IsJsonFile(fileName);
}

static std::string GetDefaultCubeModelPath()
{
#ifdef ACHENGINE_PLATFORM_LINUX
	return "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/models/Cube.fbx";
#else
	return "assets/models/Cube.fbx";
#endif
}

static bool DirectoryHasGameAssets(const std::string& directory, int depth = 0, int maxDepth = 8)
{
	if (depth > maxDepth)
	{
		return false;
	}

	DIR* dir = opendir(directory.c_str());
	if (!dir)
	{
		return false;
	}

	while (dirent* entry = readdir(dir))
	{
		const std::string name = entry->d_name;
		if (name == "." || name == "..")
		{
			continue;
		}

		std::string fullPath = directory;
		if (!fullPath.empty() && fullPath.back() != '/')
		{
			fullPath += '/';
		}
		fullPath += name;

		struct stat st;
		if (stat(fullPath.c_str(), &st) != 0)
		{
			continue;
		}

		if (S_ISDIR(st.st_mode))
		{
			if (DirectoryHasGameAssets(fullPath, depth + 1, maxDepth))
			{
				closedir(dir);
				return true;
			}
		}
		else if (IsGameAssetFile(name))
		{
			closedir(dir);
			return true;
		}
	}

	closedir(dir);
	return false;
}

struct FBrowserEntry
{
	std::string Name;
	std::string FullPath;
	bool IsDirectory = false;
};

static std::vector<FBrowserEntry> ReadDirectoryEntries(const std::string& directory)
{
	std::vector<FBrowserEntry> entries;
	DIR* dir = opendir(directory.c_str());
	if (!dir)
	{
		return entries;
	}

	while (dirent* entry = readdir(dir))
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
		{
			continue;
		}

		std::string fullPath = directory;
		if (!fullPath.empty() && fullPath.back() != '/')
		{
			fullPath += '/';
		}
		fullPath += name;

		struct stat st;
		if (stat(fullPath.c_str(), &st) != 0)
		{
			continue;
		}

		FBrowserEntry browserEntry;
		browserEntry.Name = name;
		browserEntry.FullPath = fullPath;
		browserEntry.IsDirectory = S_ISDIR(st.st_mode) != 0;
		if (browserEntry.IsDirectory)
		{
			if (DirectoryHasGameAssets(browserEntry.FullPath))
			{
				entries.push_back(browserEntry);
			}
		}
		else if (IsGameAssetFile(browserEntry.Name))
		{
			entries.push_back(browserEntry);
		}
	}

	closedir(dir);

	std::sort(entries.begin(), entries.end(), [](const FBrowserEntry& a, const FBrowserEntry& b) {
		if (a.IsDirectory != b.IsDirectory)
		{
			return a.IsDirectory > b.IsDirectory;
		}
		return a.Name < b.Name;
	});

	return entries;
}

static void CollectModelFilesRecursive(const std::string& directory, std::vector<std::string>& outFiles, int depth = 0, int maxDepth = 4)
{
	if (depth > maxDepth)
	{
		return;
	}

	DIR* dir = opendir(directory.c_str());
	if (!dir)
	{
		return;
	}

	while (dirent* entry = readdir(dir))
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
		{
			continue;
		}

		std::string fullPath = directory;
		if (!fullPath.empty() && fullPath.back() != '/')
		{
			fullPath += '/';
		}
		fullPath += name;

		struct stat st;
		if (stat(fullPath.c_str(), &st) != 0)
		{
			continue;
		}

		if (S_ISDIR(st.st_mode))
		{
			CollectModelFilesRecursive(fullPath, outFiles, depth + 1, maxDepth);
		}
		else if (IsModelFile(name))
		{
			outFiles.push_back(fullPath);
		}
	}

	closedir(dir);
}

static std::string ToDisplayRelativePath(const std::string& path, const std::string& root)
{
	if (!root.empty() && path.find(root) == 0)
	{
		std::string rel = path.substr(root.size());
		if (!rel.empty() && (rel[0] == '/' || rel[0] == '\\'))
		{
			rel = rel.substr(1);
		}
		return rel;
	}

	return path;
}

namespace
{
	struct FJsonValue
	{
		enum class EType
		{
			Null,
			Bool,
			Number,
			String,
			Object,
			Array
		};

		EType Type = EType::Null;
		bool BoolValue = false;
		double NumberValue = 0.0;
		std::string StringValue;
		std::unordered_map<std::string, FJsonValue> ObjectValue;
		std::vector<FJsonValue> ArrayValue;
	};

	class FJsonParser
	{
	public:
		explicit FJsonParser(const std::string& source)
			: m_Source(source)
		{
		}

		bool Parse(FJsonValue& outValue)
		{
			SkipWhitespace();
			if (!ParseValue(outValue))
			{
				return false;
			}

			SkipWhitespace();
			return m_Index == m_Source.size();
		}

	private:
		void SkipWhitespace()
		{
			while (m_Index < m_Source.size() && std::isspace((unsigned char)m_Source[m_Index]))
			{
				++m_Index;
			}
		}

		bool MatchToken(const char* token)
		{
			const size_t tokenLen = std::strlen(token);
			if (m_Index + tokenLen > m_Source.size())
			{
				return false;
			}

			if (m_Source.compare(m_Index, tokenLen, token) != 0)
			{
				return false;
			}

			m_Index += tokenLen;
			return true;
		}

		bool ParseString(std::string& outString)
		{
			if (m_Index >= m_Source.size() || m_Source[m_Index] != '"')
			{
				return false;
			}

			++m_Index;
			std::string result;
			while (m_Index < m_Source.size())
			{
				const char c = m_Source[m_Index++];
				if (c == '"')
				{
					outString = result;
					return true;
				}
				if (c == '\\')
				{
					if (m_Index >= m_Source.size())
					{
						return false;
					}

					const char esc = m_Source[m_Index++];
					switch (esc)
					{
						case '"': result.push_back('"'); break;
						case '\\': result.push_back('\\'); break;
						case '/': result.push_back('/'); break;
						case 'b': result.push_back('\b'); break;
						case 'f': result.push_back('\f'); break;
						case 'n': result.push_back('\n'); break;
						case 'r': result.push_back('\r'); break;
						case 't': result.push_back('\t'); break;
						default: return false;
					}
				}
				else
				{
					result.push_back(c);
				}
			}

			return false;
		}

		bool ParseNumber(double& outNumber)
		{
			const size_t start = m_Index;
			if (m_Index < m_Source.size() && (m_Source[m_Index] == '-' || m_Source[m_Index] == '+'))
			{
				++m_Index;
			}

			bool hasDigits = false;
			while (m_Index < m_Source.size() && std::isdigit((unsigned char)m_Source[m_Index]))
			{
				hasDigits = true;
				++m_Index;
			}

			if (m_Index < m_Source.size() && m_Source[m_Index] == '.')
			{
				++m_Index;
				while (m_Index < m_Source.size() && std::isdigit((unsigned char)m_Source[m_Index]))
				{
					hasDigits = true;
					++m_Index;
				}
			}

			if (m_Index < m_Source.size() && (m_Source[m_Index] == 'e' || m_Source[m_Index] == 'E'))
			{
				++m_Index;
				if (m_Index < m_Source.size() && (m_Source[m_Index] == '+' || m_Source[m_Index] == '-'))
				{
					++m_Index;
				}
				while (m_Index < m_Source.size() && std::isdigit((unsigned char)m_Source[m_Index]))
				{
					hasDigits = true;
					++m_Index;
				}
			}

			if (!hasDigits)
			{
				return false;
			}

			outNumber = std::strtod(m_Source.substr(start, m_Index - start).c_str(), nullptr);
			return true;
		}

		bool ParseArray(FJsonValue& outValue)
		{
			if (m_Source[m_Index] != '[')
			{
				return false;
			}

			++m_Index;
			SkipWhitespace();
			outValue.Type = FJsonValue::EType::Array;
			if (m_Index < m_Source.size() && m_Source[m_Index] == ']')
			{
				++m_Index;
				return true;
			}

			while (m_Index < m_Source.size())
			{
				FJsonValue element;
				if (!ParseValue(element))
				{
					return false;
				}
				outValue.ArrayValue.push_back(element);

				SkipWhitespace();
				if (m_Index < m_Source.size() && m_Source[m_Index] == ',')
				{
					++m_Index;
					SkipWhitespace();
					continue;
				}

				if (m_Index < m_Source.size() && m_Source[m_Index] == ']')
				{
					++m_Index;
					return true;
				}

				return false;
			}

			return false;
		}

		bool ParseObject(FJsonValue& outValue)
		{
			if (m_Source[m_Index] != '{')
			{
				return false;
			}

			++m_Index;
			SkipWhitespace();
			outValue.Type = FJsonValue::EType::Object;
			if (m_Index < m_Source.size() && m_Source[m_Index] == '}')
			{
				++m_Index;
				return true;
			}

			while (m_Index < m_Source.size())
			{
				std::string key;
				if (!ParseString(key))
				{
					return false;
				}

				SkipWhitespace();
				if (m_Index >= m_Source.size() || m_Source[m_Index] != ':')
				{
					return false;
				}
				++m_Index;
				SkipWhitespace();

				FJsonValue value;
				if (!ParseValue(value))
				{
					return false;
				}
				outValue.ObjectValue[key] = value;

				SkipWhitespace();
				if (m_Index < m_Source.size() && m_Source[m_Index] == ',')
				{
					++m_Index;
					SkipWhitespace();
					continue;
				}

				if (m_Index < m_Source.size() && m_Source[m_Index] == '}')
				{
					++m_Index;
					return true;
				}

				return false;
			}

			return false;
		}

		bool ParseValue(FJsonValue& outValue)
		{
			SkipWhitespace();
			if (m_Index >= m_Source.size())
			{
				return false;
			}

			const char c = m_Source[m_Index];
			if (c == '{')
			{
				return ParseObject(outValue);
			}
			if (c == '[')
			{
				return ParseArray(outValue);
			}
			if (c == '"')
			{
				outValue.Type = FJsonValue::EType::String;
				return ParseString(outValue.StringValue);
			}

			if (MatchToken("true"))
			{
				outValue.Type = FJsonValue::EType::Bool;
				outValue.BoolValue = true;
				return true;
			}
			if (MatchToken("false"))
			{
				outValue.Type = FJsonValue::EType::Bool;
				outValue.BoolValue = false;
				return true;
			}
			if (MatchToken("null"))
			{
				outValue.Type = FJsonValue::EType::Null;
				return true;
			}

			double number = 0.0;
			if (ParseNumber(number))
			{
				outValue.Type = FJsonValue::EType::Number;
				outValue.NumberValue = number;
				return true;
			}

			return false;
		}

	private:
		const std::string& m_Source;
		size_t m_Index = 0;
	};

	static std::string JsonEscape(const std::string& value)
	{
		std::string out;
		out.reserve(value.size() + 8);
		for (char c : value)
		{
			switch (c)
			{
				case '"': out += "\\\""; break;
				case '\\': out += "\\\\"; break;
				case '\n': out += "\\n"; break;
				case '\r': out += "\\r"; break;
				case '\t': out += "\\t"; break;
				default: out.push_back(c); break;
			}
		}

		return out;
	}

	static std::string JsonVec3(const glm::vec3& value)
	{
		return Achengine::format("[%.6f, %.6f, %.6f]", value.x, value.y, value.z);
	}

	static std::string ResolvePathRelativeToFile(const std::string& sourceFilePath, const std::string& candidatePath);

	static bool JsonReadObjectField(const FJsonValue& object, const std::string& key, FJsonValue& outValue)
	{
		if (object.Type != FJsonValue::EType::Object)
		{
			return false;
		}

		auto it = object.ObjectValue.find(key);
		if (it == object.ObjectValue.end())
		{
			return false;
		}

		outValue = it->second;
		return true;
	}

	static bool JsonReadStringField(const FJsonValue& object, const std::string& key, std::string& outValue)
	{
		FJsonValue field;
		if (!JsonReadObjectField(object, key, field) || field.Type != FJsonValue::EType::String)
		{
			return false;
		}

		outValue = field.StringValue;
		return true;
	}

	static bool JsonReadNumberField(const FJsonValue& object, const std::string& key, float& outValue)
	{
		FJsonValue field;
		if (!JsonReadObjectField(object, key, field) || field.Type != FJsonValue::EType::Number)
		{
			return false;
		}

		outValue = (float)field.NumberValue;
		return true;
	}

	static bool JsonReadVec3Field(const FJsonValue& object, const std::string& key, glm::vec3& outValue)
	{
		FJsonValue field;
		if (!JsonReadObjectField(object, key, field) || field.Type != FJsonValue::EType::Array || field.ArrayValue.size() != 3)
		{
			return false;
		}

		for (size_t i = 0; i < 3; ++i)
		{
			if (field.ArrayValue[i].Type != FJsonValue::EType::Number)
			{
				return false;
			}
		}

		outValue = glm::vec3((float)field.ArrayValue[0].NumberValue, (float)field.ArrayValue[1].NumberValue, (float)field.ArrayValue[2].NumberValue);
		return true;
	}

	static bool JsonReadIntField(const FJsonValue& object, const std::string& key, int& outValue)
	{
		FJsonValue field;
		if (!JsonReadObjectField(object, key, field) || field.Type != FJsonValue::EType::Number)
		{
			return false;
		}

		outValue = (int)field.NumberValue;
		return true;
	}

	static std::string GetComponentTypeTag(const Achengine::UActorComponent* component)
	{
		if (!component)
		{
			return "component";
		}

		if (dynamic_cast<const Achengine::UWaterMesh*>(component))
		{
			return "water";
		}
		if (dynamic_cast<const Achengine::UMesh*>(component))
		{
			return "mesh";
		}
		if (dynamic_cast<const Achengine::ULightComponent*>(component))
		{
			return "light";
		}
		if (dynamic_cast<const Achengine::UCameraComponent*>(component))
		{
			return "camera";
		}

		return "component";
	}

	static bool ComponentMatchesTypeTag(const Achengine::UActorComponent* component, const std::string& typeTag)
	{
		if (typeTag == "component" || typeTag.empty())
		{
			return component != nullptr;
		}

		return GetComponentTypeTag(component) == typeTag;
	}

	static Achengine::UActorComponent* CreateComponentFromTypeTag(const std::string& typeTag)
	{
		if (typeTag == "mesh")
		{
			return new Achengine::UMesh();
		}
		if (typeTag == "water")
		{
			return new Achengine::UWaterMesh();
		}
		if (typeTag == "light")
		{
			return new Achengine::ULightComponent();
		}
		if (typeTag == "camera")
		{
			return new Achengine::UCameraComponent();
		}

		return nullptr;
	}

#include "ComponentReflection.inl"

	enum class EActorTemplateLoadResult
	{
		Spawned,
		NotTemplate,
		Failed
	};

	static std::string ResolvePathRelativeToFile(const std::string& sourceFilePath, const std::string& candidatePath)
	{
		if (candidatePath.empty())
		{
			return candidatePath;
		}

		if (candidatePath[0] == '/' || candidatePath[0] == '\\')
		{
			return candidatePath;
		}

		if (candidatePath.size() > 1 && candidatePath[1] == ':')
		{
			return candidatePath;
		}

		std::string baseDir = GetDirectoryFromPath(sourceFilePath);
		if (baseDir.empty() || baseDir == ".")
		{
			return candidatePath;
		}

		if (baseDir.back() != '/' && baseDir.back() != '\\')
		{
			baseDir += '/';
		}

		return baseDir + candidatePath;
	}

	static std::string NormalizeActorTemplatePathForMap(const std::string& mapFilePath, const std::string& templatePath)
	{
		if (templatePath.empty())
		{
			return templatePath;
		}

		std::string normalizedTemplatePath = templatePath;
		std::replace(normalizedTemplatePath.begin(), normalizedTemplatePath.end(), '\\', '/');
		if (normalizedTemplatePath.rfind("../actors/", 0) == 0)
		{
			return normalizedTemplatePath;
		}

		const std::string actorsMarker = "/assets/actors/";
		const size_t markerPos = normalizedTemplatePath.find(actorsMarker);
		if (markerPos != std::string::npos)
		{
			return "../actors/" + normalizedTemplatePath.substr(markerPos + actorsMarker.size());
		}

		if (mapFilePath.empty())
		{
			return templatePath;
		}

		std::string resolvedTemplatePath = ResolvePathRelativeToFile(mapFilePath, templatePath);
		std::replace(resolvedTemplatePath.begin(), resolvedTemplatePath.end(), '\\', '/');

		std::string mapDirectory = GetDirectoryFromPath(mapFilePath);
		std::replace(mapDirectory.begin(), mapDirectory.end(), '\\', '/');
		const std::string mapsMarker = "/assets/maps/";
		const size_t mapsPos = mapDirectory.find(mapsMarker);
		if (mapsPos != std::string::npos)
		{
			const std::string assetsDirectory = mapDirectory.substr(0, mapsPos + std::strlen("/assets/"));
			const std::string actorsDirectory = assetsDirectory + "actors/";
			if (resolvedTemplatePath.rfind(actorsDirectory, 0) == 0)
			{
				return "../actors/" + resolvedTemplatePath.substr(actorsDirectory.size());
			}
		}

		return templatePath;
	}

	static bool ParseJsonFile(const std::string& filePath, FJsonValue& outRoot, std::string& outError)
	{
		std::ifstream in(filePath);
		if (!in.is_open())
		{
			outError = "Failed to open JSON file";
			return false;
		}

		std::stringstream buffer;
		buffer << in.rdbuf();
		in.close();

		std::string jsonText = buffer.str();
		if (jsonText.size() >= 3 &&
			(unsigned char)jsonText[0] == 0xEF &&
			(unsigned char)jsonText[1] == 0xBB &&
			(unsigned char)jsonText[2] == 0xBF)
		{
			jsonText = jsonText.substr(3);
		}

		FJsonParser parser(jsonText);
		if (!parser.Parse(outRoot) || outRoot.Type != FJsonValue::EType::Object)
		{
			outError = "JSON parse failed";
			return false;
		}

		return true;
	}

	enum class EJsonAssetType
	{
		Unknown,
		Map,
		ActorTemplate
	};

	static EJsonAssetType ClassifyJsonAssetFile(const std::string& filePath)
	{
		FJsonValue root;
		std::string parseError;
		if (!ParseJsonFile(filePath, root, parseError))
		{
			return EJsonAssetType::Unknown;
		}

		std::string templateName;
		FJsonValue componentsValue;
		if (JsonReadStringField(root, "templateName", templateName) &&
			JsonReadObjectField(root, "components", componentsValue) &&
			componentsValue.Type == FJsonValue::EType::Array)
		{
			return EJsonAssetType::ActorTemplate;
		}

		FJsonValue actorsValue;
		if (JsonReadObjectField(root, "actors", actorsValue) &&
			actorsValue.Type == FJsonValue::EType::Array)
		{
			return EJsonAssetType::Map;
		}

		return EJsonAssetType::Unknown;
	}

	struct FTemplateEditorComponent
	{
		std::string Type;

		std::string ModelPath = "../models/Cube.fbx";

		bool HasTransform = false;
		glm::vec3 TransformLocation = glm::vec3(0.0f);
		glm::vec3 TransformRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		float TransformRotationAngle = 0.0f;
		glm::vec3 TransformScale = glm::vec3(1.0f);

		bool HasLight = false;
		glm::vec3 LightColor = glm::vec3(1.0f, 0.95f, 0.85f);
		glm::vec3 LightAmbient = glm::vec3(0.15f, 0.14f, 0.12f);
		glm::vec3 LightDiffuse = glm::vec3(0.9f, 0.8f, 0.7f);
		glm::vec3 LightSpecular = glm::vec3(1.0f, 0.95f, 0.9f);
		float LightConstant = 1.0f;
		float LightLinear = 0.022f;
		float LightQuadratic = 0.0019f;

		bool HasCamera = false;
		float CameraFov = 90.0f;
		float CameraNearClip = 0.1f;
		float CameraFarClip = 1000.0f;
	};

	struct FActorTemplateEditorState
	{
		bool IsActive = false;
		std::string FilePath;
		char TemplateName[128] = {};
		std::vector<FTemplateEditorComponent> Components;
		int SelectedComponentIndex = -1;
		std::string Status;
	};

	static FActorTemplateEditorState g_ActorTemplateEditor;

	static void ResetActorTemplateEditor()
	{
		g_ActorTemplateEditor = FActorTemplateEditorState();
	}

	static bool LoadActorTemplateEditorFromFile(const std::string& filePath, std::string& outError)
	{
		FJsonValue root;
		if (!ParseJsonFile(filePath, root, outError))
		{
			return false;
		}

		std::string templateName;
		FJsonValue componentsValue;
		if (!JsonReadStringField(root, "templateName", templateName) ||
			!JsonReadObjectField(root, "components", componentsValue) ||
			componentsValue.Type != FJsonValue::EType::Array)
		{
			outError = "JSON is not an actor template";
			return false;
		}

		ResetActorTemplateEditor();
		g_ActorTemplateEditor.IsActive = true;
		g_ActorTemplateEditor.FilePath = filePath;
		std::strncpy(g_ActorTemplateEditor.TemplateName, templateName.c_str(), sizeof(g_ActorTemplateEditor.TemplateName) - 1);
		g_ActorTemplateEditor.TemplateName[sizeof(g_ActorTemplateEditor.TemplateName) - 1] = '\0';
		g_ActorTemplateEditor.Status = "Template loaded";

		for (const FJsonValue& componentValue : componentsValue.ArrayValue)
		{
			if (componentValue.Type != FJsonValue::EType::Object)
			{
				continue;
			}

			FTemplateEditorComponent component;
			if (!JsonReadStringField(componentValue, "type", component.Type) || component.Type.empty())
			{
				component.Type = "unknown";
			}

			if (component.Type == "mesh")
			{
				JsonReadStringField(componentValue, "modelPath", component.ModelPath);
			}

			if (component.Type == "transform")
			{
				component.HasTransform = true;
				JsonReadVec3Field(componentValue, "location", component.TransformLocation);
				JsonReadVec3Field(componentValue, "rotationAxis", component.TransformRotationAxis);
				JsonReadNumberField(componentValue, "rotationAngle", component.TransformRotationAngle);
				JsonReadVec3Field(componentValue, "scale", component.TransformScale);
			}

			FJsonValue transformValue;
			if (JsonReadObjectField(componentValue, "transform", transformValue) && transformValue.Type == FJsonValue::EType::Object)
			{
				component.HasTransform = true;
				JsonReadVec3Field(transformValue, "location", component.TransformLocation);
				JsonReadVec3Field(transformValue, "rotationAxis", component.TransformRotationAxis);
				JsonReadNumberField(transformValue, "rotationAngle", component.TransformRotationAngle);
				JsonReadVec3Field(transformValue, "scale", component.TransformScale);
			}

			FJsonValue lightValue;
			if (JsonReadObjectField(componentValue, "light", lightValue) && lightValue.Type == FJsonValue::EType::Object)
			{
				component.HasLight = true;
				JsonReadVec3Field(lightValue, "color", component.LightColor);
				JsonReadVec3Field(lightValue, "ambient", component.LightAmbient);
				JsonReadVec3Field(lightValue, "diffuse", component.LightDiffuse);
				JsonReadVec3Field(lightValue, "specular", component.LightSpecular);
				JsonReadNumberField(lightValue, "constant", component.LightConstant);
				JsonReadNumberField(lightValue, "linear", component.LightLinear);
				JsonReadNumberField(lightValue, "quadratic", component.LightQuadratic);
			}

			FJsonValue cameraValue;
			if (JsonReadObjectField(componentValue, "camera", cameraValue) && cameraValue.Type == FJsonValue::EType::Object)
			{
				component.HasCamera = true;
				JsonReadNumberField(cameraValue, "fov", component.CameraFov);
				JsonReadNumberField(cameraValue, "nearClip", component.CameraNearClip);
				JsonReadNumberField(cameraValue, "farClip", component.CameraFarClip);
			}

			if (glm::length(component.TransformRotationAxis) < 0.0001f)
			{
				component.TransformRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
			}

			g_ActorTemplateEditor.Components.push_back(component);
		}

		if (!g_ActorTemplateEditor.Components.empty())
		{
			g_ActorTemplateEditor.SelectedComponentIndex = 0;
		}

		return true;
	}

	static bool SaveActorTemplateEditorToFile(std::string& outError)
	{
		if (!g_ActorTemplateEditor.IsActive || g_ActorTemplateEditor.FilePath.empty())
		{
			outError = "No active actor template selected";
			return false;
		}

		const std::string templateName = g_ActorTemplateEditor.TemplateName;
		if (templateName.empty())
		{
			outError = "Template name cannot be empty";
			return false;
		}

		std::ostringstream out;
		out << "{\n";
		out << "  \"templateName\": \"" << JsonEscape(templateName) << "\",\n";
		out << "  \"components\": [\n";

		for (size_t i = 0; i < g_ActorTemplateEditor.Components.size(); ++i)
		{
			const FTemplateEditorComponent& component = g_ActorTemplateEditor.Components[i];
			out << "    {\n";
			out << "      \"type\": \"" << JsonEscape(component.Type) << "\"";

			if (component.Type == "mesh")
			{
				out << ",\n      \"modelPath\": \"" << JsonEscape(component.ModelPath) << "\"";
			}

			if (component.Type == "transform" && component.HasTransform)
			{
				out << ",\n      \"location\": " << JsonVec3(component.TransformLocation);
				out << ",\n      \"rotationAxis\": " << JsonVec3(component.TransformRotationAxis);
				out << ",\n      \"rotationAngle\": " << component.TransformRotationAngle;
				out << ",\n      \"scale\": " << JsonVec3(component.TransformScale);
			}
			else if (component.HasTransform)
			{
				out << ",\n      \"transform\": {\n";
				out << "        \"location\": " << JsonVec3(component.TransformLocation) << ",\n";
				out << "        \"rotationAxis\": " << JsonVec3(component.TransformRotationAxis) << ",\n";
				out << "        \"rotationAngle\": " << component.TransformRotationAngle << ",\n";
				out << "        \"scale\": " << JsonVec3(component.TransformScale) << "\n";
				out << "      }";
			}

			if (component.HasLight)
			{
				out << ",\n      \"light\": {\n";
				out << "        \"color\": " << JsonVec3(component.LightColor) << ",\n";
				out << "        \"ambient\": " << JsonVec3(component.LightAmbient) << ",\n";
				out << "        \"diffuse\": " << JsonVec3(component.LightDiffuse) << ",\n";
				out << "        \"specular\": " << JsonVec3(component.LightSpecular) << ",\n";
				out << "        \"constant\": " << component.LightConstant << ",\n";
				out << "        \"linear\": " << component.LightLinear << ",\n";
				out << "        \"quadratic\": " << component.LightQuadratic << "\n";
				out << "      }";
			}

			if (component.HasCamera)
			{
				out << ",\n      \"camera\": {\n";
				out << "        \"fov\": " << component.CameraFov << ",\n";
				out << "        \"nearClip\": " << component.CameraNearClip << ",\n";
				out << "        \"farClip\": " << component.CameraFarClip << "\n";
				out << "      }";
			}

			out << "\n    }";
			if (i + 1 < g_ActorTemplateEditor.Components.size())
			{
				out << ",";
			}
			out << "\n";
		}

		out << "  ]\n";
		out << "}\n";

		return WriteTextFileAtomically(g_ActorTemplateEditor.FilePath, out.str(), outError);
	}

	static EActorTemplateLoadResult SpawnActorTemplateFromJson(const std::string& filePath, const glm::vec3& dropLocation, std::string& outStatus, Achengine::AActor** outSpawnedActor = nullptr, const std::string& templateType = "")
	{
		if (outSpawnedActor)
		{
			*outSpawnedActor = nullptr;
		}

		FJsonValue root;
		std::string parseError;
		if (!ParseJsonFile(filePath, root, parseError))
		{
			outStatus = Achengine::format("Template JSON error: %s", parseError.c_str());
			return EActorTemplateLoadResult::Failed;
		}

		std::string templateName;
		FJsonValue componentsValue;
		if (!JsonReadStringField(root, "templateName", templateName) ||
			!JsonReadObjectField(root, "components", componentsValue) ||
			componentsValue.Type != FJsonValue::EType::Array)
		{
			return EActorTemplateLoadResult::NotTemplate;
		}

		bool spawnAsPlayerStart = false;
		for (const FJsonValue& componentValue : componentsValue.ArrayValue)
		{
			if (componentValue.Type != FJsonValue::EType::Object)
			{
				continue;
			}

			std::string componentType;
			if (JsonReadStringField(componentValue, "type", componentType) && componentType == "playerStart")
			{
				spawnAsPlayerStart = true;
				break;
			}
		}

		Achengine::AActor* actor = spawnAsPlayerStart
			? static_cast<Achengine::AActor*>(Achengine::WorldActorCache::SpawnActor<Achengine::APlayerStart>())
			: Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();

		if (!templateName.empty())
		{
			actor->SetActorName(templateName);
		}
		actor->SetTemplateType(templateType.empty() ? filePath : templateType);

		glm::vec3 localOffset(0.0f);
		glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
		float rotationAngle = 0.0f;
		glm::vec3 scale(1.0f);

		for (const FJsonValue& componentValue : componentsValue.ArrayValue)
		{
			if (componentValue.Type != FJsonValue::EType::Object)
			{
				continue;
			}

			std::string componentType;
			if (!JsonReadStringField(componentValue, "type", componentType))
			{
				continue;
			}

			if (componentType == "transform")
			{
				JsonReadVec3Field(componentValue, "location", localOffset);
				JsonReadVec3Field(componentValue, "rotationAxis", rotationAxis);
				JsonReadNumberField(componentValue, "rotationAngle", rotationAngle);
				JsonReadVec3Field(componentValue, "scale", scale);
			}
			else if (componentType == "mesh")
			{
				std::string modelPath;
				JsonReadStringField(componentValue, "modelPath", modelPath);
				if (!modelPath.empty())
				{
					modelPath = ResolvePathRelativeToFile(filePath, modelPath);
				}

				if (modelPath.empty() || !FileExists(modelPath))
				{
					modelPath = GetDefaultCubeModelPath();
				}

				if (FileExists(modelPath))
				{
					actor->AddActorComponent(new Achengine::UMesh(modelPath));
				}
			}
			else if (componentType == "water")
			{
				actor->AddActorComponent(new Achengine::UWaterMesh());
			}
			else if (componentType == "camera")
			{
				Achengine::UCameraComponent* cameraComponent = new Achengine::UCameraComponent();

				FJsonValue componentTransformValue;
				if (JsonReadObjectField(componentValue, "transform", componentTransformValue) && componentTransformValue.Type == FJsonValue::EType::Object)
				{
					glm::vec3 cameraRelativeLocation(0.0f);
					glm::vec3 cameraRelativeRotationAxis(1.0f, 0.0f, 0.0f);
					float cameraRelativeRotationAngle = 0.0f;

					JsonReadVec3Field(componentTransformValue, "location", cameraRelativeLocation);
					JsonReadVec3Field(componentTransformValue, "rotationAxis", cameraRelativeRotationAxis);
					JsonReadNumberField(componentTransformValue, "rotationAngle", cameraRelativeRotationAngle);

					if (glm::length(cameraRelativeRotationAxis) < 0.0001f)
					{
						cameraRelativeRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					cameraComponent->SetRelativeLocation(cameraRelativeLocation);
					cameraComponent->SetRelativeRotation(glm::normalize(cameraRelativeRotationAxis), cameraRelativeRotationAngle);
				}

				FJsonValue cameraValue;
				if (JsonReadObjectField(componentValue, "camera", cameraValue) && cameraValue.Type == FJsonValue::EType::Object)
				{
					float fov = cameraComponent->GetFieldOfView();
					float nearClip = 0.1f;
					float farClip = 1000.0f;

					JsonReadNumberField(cameraValue, "fov", fov);
					JsonReadNumberField(cameraValue, "nearClip", nearClip);
					JsonReadNumberField(cameraValue, "farClip", farClip);

					if (nearClip < 0.001f)
					{
						nearClip = 0.001f;
					}
					if (farClip <= nearClip)
					{
						farClip = nearClip + 1.0f;
					}

					cameraComponent->SetFieldOfView(fov);
					cameraComponent->SetClipPlanes(nearClip, farClip);
					cameraComponent->UpdateProjection();
				}

				actor->AddActorComponent(cameraComponent);
			}
			else if (componentType == "light")
			{
				Achengine::ULightComponent* lightComponent = new Achengine::ULightComponent();
				FJsonValue lightValue;
				if (JsonReadObjectField(componentValue, "light", lightValue) && lightValue.Type == FJsonValue::EType::Object)
				{
					if (Achengine::FLightSource* ls = lightComponent->GetLightSource())
					{
						JsonReadVec3Field(lightValue, "color", ls->color);
						JsonReadVec3Field(lightValue, "ambient", ls->ambient);
						JsonReadVec3Field(lightValue, "diffuse", ls->diffuse);
						JsonReadVec3Field(lightValue, "specular", ls->specular);
						JsonReadNumberField(lightValue, "constant", ls->constant);
						JsonReadNumberField(lightValue, "linear", ls->linear);
						JsonReadNumberField(lightValue, "quadratic", ls->quadratic);
					}
				}

				actor->AddActorComponent(lightComponent);
			}
		}

		if (glm::length(rotationAxis) < 0.0001f)
		{
			rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		actor->SetActorLocation(dropLocation + localOffset);
		actor->SetActorRotation(glm::normalize(rotationAxis), rotationAngle);
		actor->SetActorScale(scale);

		if (outSpawnedActor)
		{
			*outSpawnedActor = actor;
		}

		outStatus = Achengine::format("Spawned template '%s'", templateName.empty() ? "Unnamed" : templateName.c_str());
		return EActorTemplateLoadResult::Spawned;
	}
}

Sandbox3D::Sandbox3D()
	: Layer("Sandbox3D")
{
	m_CameraController = new Achengine::EditorCameraController(1280.0f / 720.0f);
}

Sandbox3D::~Sandbox3D()
{
	delete m_PlayerController;
	delete m_CameraController;
}

void Sandbox3D::OnAttach()
{
#ifdef ACHENGINE_PLATFORM_LINUX
	const std::string modelPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/models/sample.fbx";
	const std::string mapPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/maps/default_map.json";
#else
	const std::string modelPath = "assets/models/sample.fbx";
	const std::string mapPath = "assets/maps/default_map.json";
#endif

	std::strncpy(m_ModelPathBuffer, modelPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
	m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
	std::strncpy(m_MapPathBuffer, mapPath.c_str(), sizeof(m_MapPathBuffer) - 1);
	m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';

	if (FileExists(m_MapPathBuffer))
	{
		LoadMapFromFile(m_MapPathBuffer);
		ACHENGINE_CORE_INFO(m_MapStatus);
	}
	else
	{
		m_MapStatus = "No map found at " + std::string(m_MapPathBuffer) + ". Creating default scene.";
		SpawnDefaultScene();
	}
}

void Sandbox3D::EnsurePlaySessionActorPossession()
{
	if (!m_IsPlaying)
	{
		return;
	}

	if (!m_PlayerController)
	{
		m_PlayerController = new Achengine::APlayerController();
	}

	m_PlayerActor = nullptr;
	std::vector<Achengine::APlayerStart*> playerStarts;
	if (Achengine::WorldActorCache* cache = Achengine::WorldActorCache::Get())
	{
		for (Achengine::AActor* actor : cache->GetActorCache())
		{
			if (Achengine::APlayerStart* playerStart = dynamic_cast<Achengine::APlayerStart*>(actor))
			{
				playerStarts.push_back(playerStart);
			}

			if (Achengine::APlayer* player = dynamic_cast<Achengine::APlayer*>(actor))
			{
				m_PlayerActor = player;
				break;
			}
		}
	}

	if (!m_PlayerActor)
	{
		m_PlayerActor = static_cast<Achengine::APlayer*>(Achengine::WorldActorCache::SpawnActor<Achengine::APlayer>());
		glm::vec3 spawnLocation(0.0f, 0.0f, 0.0f);
		Achengine::FRotation spawnRotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
		if (!playerStarts.empty())
		{
			static std::mt19937 rng(std::random_device{}());
			std::uniform_int_distribution<size_t> distribution(0, playerStarts.size() - 1);
			Achengine::APlayerStart* selectedStart = playerStarts[distribution(rng)];
			spawnLocation = selectedStart->GetActorLocation();
			spawnRotation = selectedStart->GetActorRotation();
		}

		m_PlayerActor->SetActorLocation(spawnLocation);
		m_PlayerActor->SetActorRotation(spawnRotation.RotationAxis, spawnRotation.Angle);
	}

	if (m_PlayerController)
	{
		m_PlayerController->Possess(m_PlayerActor);
	}
}

void Sandbox3D::StartPlayMode()
{
	if (m_IsPlaying)
	{
		EnsurePlaySessionActorPossession();
		return;
	}

	if (!m_PlayerController)
	{
		m_PlayerController = new Achengine::APlayerController();
	}

	m_SelectedActors.clear();
	m_ActiveActor = nullptr;
	m_GizmoDragging = false;
	m_GizmoActiveAxis = -1;

	m_IsPlaying = true;
	SetPlayCursorCaptured(true);
	EnsurePlaySessionActorPossession();
	m_MapStatus = "Play mode started";
}

void Sandbox3D::StopPlayMode()
{
	if (!m_IsPlaying)
	{
		return;
	}

	if (m_PlayerController)
	{
		m_PlayerController->UnPossess();
		delete m_PlayerController;
		m_PlayerController = nullptr;
	}

	if (m_PlayerActor)
	{
		if (m_ActiveActor == m_PlayerActor)
		{
			m_ActiveActor = nullptr;
		}
		m_SelectedActors.erase(m_PlayerActor);
		Achengine::WorldActorCache::DestroyActor(m_PlayerActor);
		m_PlayerActor = nullptr;
	}

	m_IsPlaying = false;
	SetPlayCursorCaptured(false);
	m_MapStatus = "Play mode stopped";
}

void Sandbox3D::SpawnDefaultScene()
{
	Achengine::AActor* actor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	actor->SetActorName(Achengine::format("Floor"));
	actor->SetTemplateType("../actors/CrateTemplate.json");
	actor->AddActorComponent(new Achengine::UMesh(GetDefaultCubeModelPath()));
	actor->SetActorLocation({0.0f, 0.0f, 0.0f});
	actor->SetActorScale({100.0f, 1.0f, 100.0f});

	Achengine::AActor* lightActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	lightActor->SetActorName(Achengine::format("Light"));
	lightActor->SetTemplateType("../actors/LightTemplate.json");
	lightActor->AddActorComponent(new Achengine::ULightComponent());
	lightActor->SetActorLocation({0.0f, 10.0f, 0.0f});
}

void Sandbox3D::OnDetach()
{
	StopPlayMode();
	SetPlayCursorCaptured(false);
	m_ActiveActor = nullptr;
	m_SelectedActors.clear();
}

void Sandbox3D::OnUpdate(Achengine::Timestep timestep)
{
	// Update
	if (m_IsPlaying)
	{
		if (!m_PlayerController || !m_PlayerController->GetPossessedPlayer())
		{
			EnsurePlaySessionActorPossession();
		}

		if (m_PlayerController)
		{
			m_PlayerController->Tick(timestep.GetSeconds());
		}

		for (Achengine::AActor* actor : Achengine::WorldActorCache::Get()->GetActorCache())
		{
			if (actor)
			{
				actor->Tick(timestep.GetSeconds());
			}
		}
	}
	else if (m_CameraController)
	{
		m_CameraController->OnUpdate(timestep);
	}

	// Render
	Achengine::Application* app = Achengine::Application::Get();
	const float windowWidth = app ? (float)app->GetWindow().GetWidth() : 1280.0f;
	const float windowHeight = app ? (float)app->GetWindow().GetHeight() : 720.0f;
	const float sceneViewportWidth = glm::max(1.0f, windowWidth - m_OutlinerWidth);
	const float sceneViewportHeight = glm::max(1.0f, windowHeight - m_TopMapPanelHeight - m_AssetBrowserHeight);
	const float sceneViewportX = 0.0f;
	const float sceneViewportY = glm::max(0.0f, m_AssetBrowserHeight);

	if (m_CameraController && !m_IsPlaying)
	{
		if (Achengine::EditorCamera* editorCamera = dynamic_cast<Achengine::EditorCamera*>(m_CameraController->GetCamera()))
		{
			editorCamera->SetViewportSize(sceneViewportWidth, sceneViewportHeight);
		}
	}

	if (m_IsPlaying && m_PlayerActor)
	{
		if (Achengine::UCameraComponent* playerCamera = m_PlayerActor->GetCameraComponent())
		{
			playerCamera->SetViewportSize(sceneViewportWidth, sceneViewportHeight);
		}
	}

	Achengine::RenderCommand::SetViewport((uint32_t)sceneViewportX, (uint32_t)sceneViewportY, (uint32_t)sceneViewportWidth, (uint32_t)sceneViewportHeight);
	Achengine::RenderCommand::SetClearColor({ 0.4f, 0.4f, 0.8f, 0.3f });
	Achengine::RenderCommand::Clear();

	Achengine::Renderer::BeginScene(GetActiveSceneCamera());
	Achengine::Renderer::EndScene();

	if (!m_SelectedActors.empty())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2.0f);
		glDisable(GL_DEPTH_TEST);
		for (Achengine::AActor* actor : m_SelectedActors)
		{
			if (actor)
			{
				actor->Draw();
			}
		}
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Restore full framebuffer viewport for ImGui rendering.
	Achengine::RenderCommand::SetViewport(0, 0, (uint32_t)windowWidth, (uint32_t)windowHeight);
}

bool Sandbox3D::SaveMapToFile(const std::string& filePath)
{
	if (filePath.empty())
	{
		m_MapStatus = "Map path is empty";
		return false;
	}

	std::stringstream out;

	out << "{\n";
	out << "  \"version\": 1,\n";
	out << "  \"actors\": [\n";

	std::vector<Achengine::AActor*> actors;
	if (Achengine::WorldActorCache* cache = Achengine::WorldActorCache::Get())
	{
		for (Achengine::AActor* actor : cache->GetActorCache())
		{
			if (!actor)
			{
				continue;
			}

			actors.push_back(actor);
		}
	}

	std::sort(actors.begin(), actors.end());
	bool wroteAnyActor = false;
	for (Achengine::AActor* actor : actors)
	{
		Achengine::UMesh* mesh = actor->GetComponentByClass<Achengine::UMesh>();
		Achengine::ULightComponent* lightComponent = actor->GetComponentByClass<Achengine::ULightComponent>();
		const std::string actorTemplatePath = actor->GetTemplateType();
		if (actorTemplatePath.empty())
		{
			continue;
		}

		if (wroteAnyActor)
		{
			out << ",\n";
		}
		wroteAnyActor = true;

		const glm::vec3 location = actor->GetActorLocation();
		const glm::vec3 scale = actor->GetActorScale();
		Achengine::FRotation rotation = actor->GetActorRotation();
		glm::vec3 rotAxis = rotation.RotationAxis;
		if (glm::length(rotAxis) < 0.0001f)
		{
			rotAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		out << "    {\n";
		out << "      \"name\": \"" << JsonEscape(actor->GetActorName()) << "\",\n";
		out << "      \"actorTemplate\": \"" << JsonEscape(actorTemplatePath) << "\",\n";
		out << "      \"location\": " << JsonVec3(location) << ",\n";
		out << "      \"rotationAxis\": " << JsonVec3(glm::normalize(rotAxis)) << ",\n";
		out << "      \"rotationAngle\": " << rotation.Angle << ",\n";
		out << "      \"scale\": " << JsonVec3(scale);

		if (lightComponent)
		{
			Achengine::FLightSource* ls = lightComponent->GetLightSource();
			if (ls)
			{
				out << ",\n      \"light\": {\n";
				out << "        \"color\": " << JsonVec3(ls->color) << ",\n";
				out << "        \"ambient\": " << JsonVec3(ls->ambient) << ",\n";
				out << "        \"diffuse\": " << JsonVec3(ls->diffuse) << ",\n";
				out << "        \"specular\": " << JsonVec3(ls->specular) << ",\n";
				out << "        \"constant\": " << ls->constant << ",\n";
				out << "        \"linear\": " << ls->linear << ",\n";
				out << "        \"quadratic\": " << ls->quadratic << "\n";
				out << "      }";
			}
		}
		if (mesh && !mesh->GetModelPath().empty())
		{
			out << ",\n      \"modelPath\": \"" << JsonEscape(mesh->GetModelPath()) << "\"";
		}

		out << ",\n      \"components\": [";
		const std::vector<Achengine::UActorComponent*>& components = actor->GetActorComponents();
		if (!components.empty())
		{
			out << "\n";
			bool wroteAnyComponent = false;
			for (size_t componentIndex = 0; componentIndex < components.size(); ++componentIndex)
			{
				Achengine::UActorComponent* component = components[componentIndex];
				if (!Achengine::UActorComponent::IsPointerAlive(component))
				{
					continue;
				}

				if (wroteAnyComponent)
				{
					out << ",\n";
				}

				wroteAnyComponent = true;
				WriteComponentOverrideJson(out, component, componentIndex);
			}

			if (wroteAnyComponent)
			{
				out << "\n      ";
			}
		}
		out << "]";

		out << "\n    }";
	}

	out << "\n";

	out << "  ]\n";
	out << "}\n";

	std::string writeError;
	if (!WriteTextFileAtomically(filePath, out.str(), writeError))
	{
		m_MapStatus = Achengine::format("Failed to save map: %s", writeError.c_str());
		return false;
	}

	m_MapStatus = Achengine::format("Saved map: %s", filePath.c_str());
	return true;
}

bool Sandbox3D::LoadMapFromFile(const std::string& filePath)
{
	if (!FileExists(filePath))
	{
		m_MapStatus = "Map file not found";
		return false;
	}

	std::ifstream in(filePath);
	if (!in.is_open())
	{
		m_MapStatus = "Failed to open map file for read";
		return false;
	}

	std::stringstream buffer;
	buffer << in.rdbuf();
	in.close();
	std::string jsonText = buffer.str();
	if (jsonText.size() >= 3 &&
		(unsigned char)jsonText[0] == 0xEF &&
		(unsigned char)jsonText[1] == 0xBB &&
		(unsigned char)jsonText[2] == 0xBF)
	{
		jsonText = jsonText.substr(3);
	}

	FJsonValue root;
	FJsonParser parser(jsonText);
	if (!parser.Parse(root) || root.Type != FJsonValue::EType::Object)
	{
		m_MapStatus = "Invalid map JSON (parse failed or root is not an object)";
		return false;
	}

	FJsonValue actorsValue;
	if (!JsonReadObjectField(root, "actors", actorsValue) || actorsValue.Type != FJsonValue::EType::Array)
	{
		m_MapStatus = "Map JSON missing actors array";
		return false;
	}

	Achengine::WorldActorCache* cache = Achengine::WorldActorCache::Get();
	if (!cache)
	{
		m_MapStatus = "World cache is unavailable";
		return false;
	}

	cache->ClearActorCache();
	m_SelectedActors.clear();
	m_ActiveActor = nullptr;
	m_ModelActor = nullptr;
	m_ModelMesh = nullptr;

	int loadedActors = 0;
	for (const FJsonValue& actorValue : actorsValue.ArrayValue)
	{
		if (actorValue.Type != FJsonValue::EType::Object)
		{
			continue;
		}

		std::string name;
		std::string actorTemplate;
		std::string modelPathOverride;
		glm::vec3 location(0.0f);
		glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
		glm::vec3 scale(1.0f);
		float rotationAngle = 0.0f;
		FJsonValue lightOverrideValue;
		FJsonValue componentOverridesValue;
		JsonReadStringField(actorValue, "name", name);
		JsonReadStringField(actorValue, "actorTemplate", actorTemplate);
		JsonReadStringField(actorValue, "modelPath", modelPathOverride);
		JsonReadVec3Field(actorValue, "location", location);
		JsonReadVec3Field(actorValue, "rotationAxis", rotationAxis);
		JsonReadNumberField(actorValue, "rotationAngle", rotationAngle);
		JsonReadVec3Field(actorValue, "scale", scale);
		JsonReadObjectField(actorValue, "light", lightOverrideValue);
		JsonReadObjectField(actorValue, "components", componentOverridesValue);

		if (actorTemplate.empty())
		{
			continue;
		}

		const std::string normalizedActorTemplate = NormalizeActorTemplatePathForMap(filePath, actorTemplate);

		Achengine::AActor* spawnedFromTemplate = nullptr;
		std::string templateStatus;
		const std::string resolvedTemplatePath = ResolvePathRelativeToFile(filePath, normalizedActorTemplate);
		const EActorTemplateLoadResult templateResult = SpawnActorTemplateFromJson(resolvedTemplatePath, location, templateStatus, &spawnedFromTemplate, normalizedActorTemplate);

		if (templateResult == EActorTemplateLoadResult::Spawned && spawnedFromTemplate)
		{
			if (!name.empty())
			{
				spawnedFromTemplate->SetActorName(name);
			}
			if (glm::length(rotationAxis) < 0.0001f)
			{
				rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
			}

			spawnedFromTemplate->SetActorLocation(location);
			spawnedFromTemplate->SetActorRotation(glm::normalize(rotationAxis), rotationAngle);
			spawnedFromTemplate->SetActorScale(scale);

			if (!modelPathOverride.empty())
			{
				if (Achengine::UMesh* meshComponent = spawnedFromTemplate->GetComponentByClass<Achengine::UMesh>())
				{
					if (!dynamic_cast<Achengine::UWaterMesh*>(meshComponent))
					{
						const std::string resolvedModelPath = ResolvePathRelativeToFile(filePath, modelPathOverride);
						if (FileExists(resolvedModelPath))
						{
							meshComponent->ReloadModel(resolvedModelPath);
						}
						else if (FileExists(modelPathOverride))
						{
							meshComponent->ReloadModel(modelPathOverride);
						}
					}
				}
			}

			if (lightOverrideValue.Type == FJsonValue::EType::Object)
			{
				if (Achengine::ULightComponent* lightComponent = spawnedFromTemplate->GetComponentByClass<Achengine::ULightComponent>())
				{
					if (Achengine::FLightSource* ls = lightComponent->GetLightSource())
					{
						JsonReadVec3Field(lightOverrideValue, "color", ls->color);
						JsonReadVec3Field(lightOverrideValue, "ambient", ls->ambient);
						JsonReadVec3Field(lightOverrideValue, "diffuse", ls->diffuse);
						JsonReadVec3Field(lightOverrideValue, "specular", ls->specular);
						JsonReadNumberField(lightOverrideValue, "constant", ls->constant);
						JsonReadNumberField(lightOverrideValue, "linear", ls->linear);
						JsonReadNumberField(lightOverrideValue, "quadratic", ls->quadratic);
					}
				}
			}

			if (componentOverridesValue.Type == FJsonValue::EType::Array)
			{
				std::vector<Achengine::UActorComponent*> availableComponents;
				for (Achengine::UActorComponent* component : spawnedFromTemplate->GetActorComponents())
				{
					availableComponents.push_back(component);
				}

				std::vector<bool> consumedComponents(availableComponents.size(), false);

				for (const FJsonValue& componentValue : componentOverridesValue.ArrayValue)
				{
					if (componentValue.Type != FJsonValue::EType::Object)
					{
						continue;
					}

					std::string componentTypeTag = "component";
					JsonReadStringField(componentValue, "type", componentTypeTag);

					Achengine::UActorComponent* targetComponent = nullptr;
					int serializedIndex = -1;
					if (JsonReadIntField(componentValue, "index", serializedIndex) &&
						serializedIndex >= 0 &&
						(size_t)serializedIndex < availableComponents.size() &&
						ComponentMatchesTypeTag(availableComponents[(size_t)serializedIndex], componentTypeTag))
					{
						targetComponent = availableComponents[(size_t)serializedIndex];
						consumedComponents[(size_t)serializedIndex] = true;
					}

					if (!targetComponent)
					{
						for (size_t i = 0; i < availableComponents.size(); ++i)
						{
							if (consumedComponents[i])
							{
								continue;
							}

							if (ComponentMatchesTypeTag(availableComponents[i], componentTypeTag))
							{
								targetComponent = availableComponents[i];
								consumedComponents[i] = true;
								break;
							}
						}
					}

					if (!targetComponent)
					{
						Achengine::UActorComponent* createdComponent = CreateComponentFromTypeTag(componentTypeTag);
						if (createdComponent)
						{
							spawnedFromTemplate->AddActorComponent(createdComponent);
							availableComponents.push_back(createdComponent);
							consumedComponents.push_back(true);
							targetComponent = createdComponent;
						}
					}

					if (targetComponent)
					{
						ApplyComponentOverrideFromJson(targetComponent, componentValue, filePath);
					}
				}
			}

			if (Achengine::UMesh* spawnedMesh = spawnedFromTemplate->GetComponentByClass<Achengine::UMesh>())
			{
				if (!dynamic_cast<Achengine::UWaterMesh*>(spawnedMesh))
				{
					m_ModelActor = spawnedFromTemplate;
					m_ModelMesh = spawnedMesh;
				}
			}

			++loadedActors;
		}
	}

	if (loadedActors == 0)
	{
		m_MapStatus = "Loaded map, but no actors were reconstructed";
		return false;
	}

	m_MapStatus = Achengine::format("Loaded map: %d actors", loadedActors);
	return true;
}

Achengine::Camera* Sandbox3D::GetActiveSceneCamera() const
{
	if (m_IsPlaying && m_PlayerActor)
	{
		return m_PlayerActor->GetCameraComponent();
	}

	if (m_CameraController)
	{
		return m_CameraController->GetCamera();
	}

	return nullptr;
}

glm::vec3 Sandbox3D::GetActiveSceneCameraPosition(const Achengine::Camera* camera) const
{
	if (!camera)
	{
		return glm::vec3(0.0f);
	}

	if (const Achengine::UCameraComponent* playerCamera = dynamic_cast<const Achengine::UCameraComponent*>(camera))
	{
		return playerCamera->GetWorldLocation();
	}

	return camera->GetPosition();
}

void Sandbox3D::RenderOutlinerPanel(const ImGuiViewport* viewport, float minOutlinerWidth, float maxOutlinerWidth)
{
	static Achengine::UActorComponent* s_ActiveComponent = nullptr;
	static std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>> s_ModelPathEditBuffers;
	static std::string s_ComponentStatus;

	const float usableAssetWidth = viewport->WorkSize.x - m_OutlinerWidth;
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + usableAssetWidth, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(m_OutlinerWidth, viewport->WorkSize.y), ImGuiCond_Always);
	ImGuiWindowFlags outlinerFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Outliner", nullptr, outlinerFlags);
	m_OutlinerWidth = ImGui::GetWindowSize().x;
	if (m_OutlinerWidth < minOutlinerWidth)
	{
		m_OutlinerWidth = minOutlinerWidth;
	}
	if (m_OutlinerWidth > maxOutlinerWidth)
	{
		m_OutlinerWidth = maxOutlinerWidth;
	}

	Achengine::WorldActorCache* Cache = Achengine::WorldActorCache::Get();
	if (!Cache)
	{
		ImGui::TextUnformatted("World cache unavailable");
		ImGui::End();
		return;
	}
	std::vector<Achengine::AActor*> actors;
	for (Achengine::AActor* actor : Cache->GetActorCache())
	{
		actors.push_back(actor);
	}
	std::sort(actors.begin(), actors.end());

	for (auto it = m_SelectedActors.begin(); it != m_SelectedActors.end();)
	{
		if (std::find(actors.begin(), actors.end(), *it) == actors.end())
		{
			it = m_SelectedActors.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (m_ActiveActor && std::find(actors.begin(), actors.end(), m_ActiveActor) == actors.end())
	{
		m_ActiveActor = nullptr;
		s_ActiveComponent = nullptr;
		ResetActorTemplateEditor();
	}
	if (!m_ActiveActor && !m_SelectedActors.empty())
	{
		m_ActiveActor = *m_SelectedActors.begin();
		s_ActiveComponent = nullptr;
		ResetActorTemplateEditor();
	}

	if (s_ActiveComponent)
	{
		if (!Achengine::UActorComponent::IsPointerAlive(s_ActiveComponent) || !m_ActiveActor || s_ActiveComponent->GetOwner() != m_ActiveActor)
		{
			s_ActiveComponent = nullptr;
		}
	}

	ImGui::Text("Actors: %d", (int)actors.size());
	ImGui::SameLine();
	ImGui::TextUnformatted("(Ctrl-click for multiselect)");
	ImGui::Separator();
	if (m_IsPlaying)
	{
		ImGui::TextUnformatted("Selection disabled while in Play mode");
	}
	ImGui::BeginDisabled(m_IsPlaying);
	ImGui::BeginChild("OutlinerActors", ImVec2(0.0f, 220.0f), true);
	Achengine::AActor* previousActiveActor = m_ActiveActor;
	for (Achengine::AActor* actor : actors)
	{
		std::string shaderName = "<none>";
		if (Achengine::UMesh* mesh = actor->GetComponentByClass<Achengine::UMesh>())
		{
			shaderName = mesh->GetShaderName();
		}

		ImGui::PushID(actor);
		const bool isSelected = m_SelectedActors.count(actor) > 0;
		std::string label = Achengine::format("%s | %s", actor->GetActorName().c_str(), shaderName.c_str());
		if (ImGui::Selectable(label.c_str(), isSelected))
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				if (isSelected)
				{
					m_SelectedActors.erase(actor);
					if (m_ActiveActor == actor)
					{
						m_ActiveActor = m_SelectedActors.empty() ? nullptr : *m_SelectedActors.begin();
						s_ActiveComponent = nullptr;
						if (m_ActiveActor)
						{
							ResetActorTemplateEditor();
						}
					}
				}
				else
				{
					m_SelectedActors.insert(actor);
					m_ActiveActor = actor;
					s_ActiveComponent = nullptr;
					ResetActorTemplateEditor();
				}
			}
			else
			{
				m_SelectedActors.clear();
				m_SelectedActors.insert(actor);
				m_ActiveActor = actor;
				s_ActiveComponent = nullptr;
				ResetActorTemplateEditor();
			}
		}
		ImGui::PopID();
	}

	if (previousActiveActor != m_ActiveActor)
	{
		s_ActiveComponent = nullptr;
	}
	ImGui::EndChild();
	ImGui::EndDisabled();

	if (m_IsPlaying)
	{
		ImGui::Separator();
		ImGui::TextUnformatted("Editor transform and gizmo controls are hidden in Play mode");
		ImGui::End();
		return;
	}

	ImGui::Separator();
	if (g_ActorTemplateEditor.IsActive)
	{
		ImGui::TextUnformatted("Selected Actor Template");
		ImGui::TextWrapped("File: %s", g_ActorTemplateEditor.FilePath.c_str());
		ImGui::InputText("Template Name", g_ActorTemplateEditor.TemplateName, sizeof(g_ActorTemplateEditor.TemplateName));

		if (ImGui::Button("Add Template Component"))
		{
			ImGui::OpenPopup("OutlinerTemplateAddComponentPopup");
		}
		if (ImGui::BeginPopup("OutlinerTemplateAddComponentPopup"))
		{
			auto addTemplateComponent = [&](const char* type) {
				FTemplateEditorComponent component;
				component.Type = type;
				component.HasTransform = std::string(type) == "camera" || std::string(type) == "transform";
				component.HasLight = std::string(type) == "light";
				component.HasCamera = std::string(type) == "camera";
				g_ActorTemplateEditor.Components.push_back(component);
				g_ActorTemplateEditor.SelectedComponentIndex = (int)g_ActorTemplateEditor.Components.size() - 1;
				g_ActorTemplateEditor.Status = Achengine::format("Added component: %s", type);
				ImGui::CloseCurrentPopup();
			};

			if (ImGui::MenuItem("mesh")) { addTemplateComponent("mesh"); }
			if (ImGui::MenuItem("water")) { addTemplateComponent("water"); }
			if (ImGui::MenuItem("light")) { addTemplateComponent("light"); }
			if (ImGui::MenuItem("camera")) { addTemplateComponent("camera"); }
			if (ImGui::MenuItem("playerStart")) { addTemplateComponent("playerStart"); }
			if (ImGui::MenuItem("transform")) { addTemplateComponent("transform"); }
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		const bool hasSelectedTemplateComponent = g_ActorTemplateEditor.SelectedComponentIndex >= 0 &&
			g_ActorTemplateEditor.SelectedComponentIndex < (int)g_ActorTemplateEditor.Components.size();
		if (ImGui::Button("Remove Template Component") && hasSelectedTemplateComponent)
		{
			g_ActorTemplateEditor.Components.erase(g_ActorTemplateEditor.Components.begin() + g_ActorTemplateEditor.SelectedComponentIndex);
			if (g_ActorTemplateEditor.Components.empty())
			{
				g_ActorTemplateEditor.SelectedComponentIndex = -1;
			}
			else if (g_ActorTemplateEditor.SelectedComponentIndex >= (int)g_ActorTemplateEditor.Components.size())
			{
				g_ActorTemplateEditor.SelectedComponentIndex = (int)g_ActorTemplateEditor.Components.size() - 1;
			}
			g_ActorTemplateEditor.Status = "Removed template component";
		}

		ImGui::BeginChild("TemplateComponents", ImVec2(0.0f, 130.0f), true);
		for (size_t i = 0; i < g_ActorTemplateEditor.Components.size(); ++i)
		{
			const std::string label = Achengine::format("%s ##templateComp_%d", g_ActorTemplateEditor.Components[i].Type.c_str(), (int)i);
			if (ImGui::Selectable(label.c_str(), g_ActorTemplateEditor.SelectedComponentIndex == (int)i))
			{
				g_ActorTemplateEditor.SelectedComponentIndex = (int)i;
			}
		}
		ImGui::EndChild();

		if (hasSelectedTemplateComponent)
		{
			FTemplateEditorComponent& component = g_ActorTemplateEditor.Components[g_ActorTemplateEditor.SelectedComponentIndex];
			ImGui::Separator();
			ImGui::Text("Editing component type: %s", component.Type.c_str());

			if (component.Type == "mesh")
			{
				char modelPath[512] = {};
				std::strncpy(modelPath, component.ModelPath.c_str(), sizeof(modelPath) - 1);
				if (ImGui::InputText("Model Path", modelPath, sizeof(modelPath)))
				{
					component.ModelPath = modelPath;
				}
			}

			if (component.Type == "transform" || component.Type == "camera")
			{
				component.HasTransform = true;
				ImGui::DragFloat3("Location", &component.TransformLocation.x, 0.1f);
				ImGui::DragFloat3("Rotation Axis", &component.TransformRotationAxis.x, 0.01f, -1.0f, 1.0f);
				if (glm::length(component.TransformRotationAxis) < 0.0001f)
				{
					component.TransformRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
				}
				ImGui::DragFloat("Rotation Angle", &component.TransformRotationAngle, 0.25f, -360.0f, 360.0f);
				ImGui::DragFloat3("Scale", &component.TransformScale.x, 0.05f, 0.01f, 1000.0f);
			}

			if (component.Type == "light")
			{
				component.HasLight = true;
				ImGui::ColorEdit3("Color", &component.LightColor.x);
				ImGui::ColorEdit3("Ambient", &component.LightAmbient.x);
				ImGui::ColorEdit3("Diffuse", &component.LightDiffuse.x);
				ImGui::ColorEdit3("Specular", &component.LightSpecular.x);
				ImGui::DragFloat("Constant", &component.LightConstant, 0.01f, 0.0f, 20.0f);
				ImGui::DragFloat("Linear", &component.LightLinear, 0.001f, 0.0f, 10.0f);
				ImGui::DragFloat("Quadratic", &component.LightQuadratic, 0.0001f, 0.0f, 10.0f);
			}

			if (component.Type == "camera")
			{
				component.HasCamera = true;
				ImGui::DragFloat("FOV", &component.CameraFov, 0.1f, 1.0f, 179.0f);
				ImGui::DragFloat("Near Clip", &component.CameraNearClip, 0.01f, 0.001f, 1000.0f);
				ImGui::DragFloat("Far Clip", &component.CameraFarClip, 1.0f, component.CameraNearClip + 0.001f, 100000.0f);
				if (component.CameraFarClip <= component.CameraNearClip)
				{
					component.CameraFarClip = component.CameraNearClip + 0.001f;
				}
			}
		}

		ImGui::Separator();
		if (ImGui::Button("Save Template JSON"))
		{
			std::string saveError;
			if (SaveActorTemplateEditorToFile(saveError))
			{
				g_ActorTemplateEditor.Status = "Template saved";
			}
			else
			{
				g_ActorTemplateEditor.Status = Achengine::format("Save failed: %s", saveError.c_str());
			}
		}

		if (!g_ActorTemplateEditor.Status.empty())
		{
			ImGui::TextWrapped("%s", g_ActorTemplateEditor.Status.c_str());
		}
	}
	else
	{
		ImGui::TextUnformatted("Selected Actor");
		if (m_ActiveActor)
		{
			ImGui::Text("Active: %s", m_ActiveActor->GetActorName().c_str());
			ImGui::Text("Selected Count: %d", (int)m_SelectedActors.size());

			glm::vec3 location = m_ActiveActor->GetActorLocation();
			if (ImGui::DragFloat3("Location", &location.x, 0.1f))
			{
				m_ActiveActor->SetActorLocation(location);
			}

			Achengine::FRotation rotation = m_ActiveActor->GetActorRotation();
			glm::vec3 rotationAxis = rotation.RotationAxis;
			float angle = rotation.Angle;
			if (ImGui::DragFloat3("Rotation Axis", &rotationAxis.x, 0.01f, -1.0f, 1.0f))
			{
				if (glm::length(rotationAxis) < 0.0001f)
				{
					rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
				}
				m_ActiveActor->SetActorRotation(glm::normalize(rotationAxis), angle);
			}
			if (ImGui::DragFloat("Rotation Angle", &angle, 0.25f, -360.0f, 360.0f))
			{
				m_ActiveActor->SetActorRotation(glm::normalize(rotationAxis), angle);
			}

			glm::vec3 scale = m_ActiveActor->GetActorScale();
			if (ImGui::DragFloat3("Scale", &scale.x, 0.05f, 0.01f, 1000.0f))
			{
				m_ActiveActor->SetActorScale(scale);
			}

			ImGui::Separator();
			ImGui::Text("Gizmo Mode");
			if (ImGui::RadioButton("Translate", m_GizmoMode == EGizmoMode::Translate))
			{
				m_GizmoMode = EGizmoMode::Translate;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Rotate", m_GizmoMode == EGizmoMode::Rotate))
			{
				m_GizmoMode = EGizmoMode::Rotate;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Scale", m_GizmoMode == EGizmoMode::Scale))
			{
				m_GizmoMode = EGizmoMode::Scale;
			}

			ImGui::Text("Gizmo Space");
			if (ImGui::RadioButton("World", m_GizmoSpace == EGizmoSpace::World))
			{
				m_GizmoSpace = EGizmoSpace::World;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Local", m_GizmoSpace == EGizmoSpace::Local))
			{
				m_GizmoSpace = EGizmoSpace::Local;
			}

			ImGui::Checkbox("Snap", &m_UseGizmoSnapping);
			if (m_GizmoMode == EGizmoMode::Translate)
			{
				ImGui::DragFloat("Translate Snap", &m_TranslateSnapStep, 0.05f, 0.01f, 100.0f);
			}
			else if (m_GizmoMode == EGizmoMode::Rotate)
			{
				ImGui::DragFloat("Rotate Snap", &m_RotateSnapStep, 0.5f, 1.0f, 180.0f);
			}
			else
			{
				ImGui::DragFloat("Scale Snap", &m_ScaleSnapStep, 0.01f, 0.01f, 10.0f);
			}
			ImGui::TextUnformatted("Drag axis handles in viewport to transform selected actors.");

			ImGui::Separator();
			ImGui::TextUnformatted("Components");
			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("OutlinerAddComponentPopup");
			}

			if (ImGui::BeginPopup("OutlinerAddComponentPopup"))
			{
				if (ImGui::MenuItem("UMesh"))
				{
					std::string defaultModelPath = GetDefaultCubeModelPath();
					if (!FileExists(defaultModelPath))
					{
						defaultModelPath.clear();
					}

					Achengine::UMesh* meshComponent = defaultModelPath.empty()
						? new Achengine::UMesh()
						: new Achengine::UMesh(defaultModelPath);
					m_ActiveActor->AddActorComponent(meshComponent);
					s_ActiveComponent = meshComponent;
					SyncModelPathBufferForComponent(s_ActiveComponent, s_ModelPathEditBuffers);
					s_ComponentStatus = "Added UMesh";
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("UWaterMesh"))
				{
					Achengine::UWaterMesh* waterMeshComponent = new Achengine::UWaterMesh();
					m_ActiveActor->AddActorComponent(waterMeshComponent);
					s_ActiveComponent = waterMeshComponent;
					s_ComponentStatus = "Added UWaterMesh";
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("ULightComponent"))
				{
					Achengine::ULightComponent* lightComponent = new Achengine::ULightComponent();
					m_ActiveActor->AddActorComponent(lightComponent);
					s_ActiveComponent = lightComponent;
					s_ComponentStatus = "Added ULightComponent";
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("UCameraComponent"))
				{
					Achengine::UCameraComponent* cameraComponent = new Achengine::UCameraComponent();
					m_ActiveActor->AddActorComponent(cameraComponent);
					s_ActiveComponent = cameraComponent;
					s_ComponentStatus = "Added UCameraComponent";
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::SameLine();
			if (s_ActiveComponent && ImGui::Button("Remove Selected Component"))
			{
				Achengine::UActorComponent* removedComponent = s_ActiveComponent;
				if (m_ActiveActor->RemoveActorComponent(removedComponent))
				{
					s_ModelPathEditBuffers.erase(removedComponent);
					s_ActiveComponent = nullptr;
					s_ComponentStatus = "Component removed";
				}
				else
				{
					s_ComponentStatus = "Failed to remove component";
				}
			}

			const std::vector<Achengine::UActorComponent*>& components = m_ActiveActor->GetActorComponents();
			if (components.empty())
			{
				ImGui::TextUnformatted("No components");
			}
			else
			{
				ImGui::BeginChild("OutlinerComponents", ImVec2(0.0f, 140.0f), true);
				for (size_t i = 0; i < components.size(); ++i)
				{
					Achengine::UActorComponent* component = components[i];
					if (!Achengine::UActorComponent::IsPointerAlive(component))
					{
						continue;
					}

					const std::string itemLabel = Achengine::format("%s ##comp_%d", GetComponentDisplayName(component), (int)i);
					if (ImGui::Selectable(itemLabel.c_str(), component == s_ActiveComponent))
					{
						s_ActiveComponent = component;
						s_ComponentStatus.clear();
						SyncModelPathBufferForComponent(s_ActiveComponent, s_ModelPathEditBuffers);
					}
				}
				ImGui::EndChild();

				if (s_ActiveComponent)
				{
					ImGui::Separator();
					ImGui::TextUnformatted("Component Properties");
					ImGui::PushID(s_ActiveComponent);
					DrawDefaultComponentTransformProperties(s_ActiveComponent);
					DrawReflectedComponentProperties(s_ActiveComponent, s_ModelPathEditBuffers, s_ComponentStatus);
					ImGui::PopID();

					if (!s_ComponentStatus.empty())
					{
						ImGui::TextWrapped("%s", s_ComponentStatus.c_str());
					}
				}
			}
		}
		else
		{
			ImGui::TextUnformatted("No actor or template selected");
			s_ActiveComponent = nullptr;
		}
	}

	ImGui::End();
}

void Sandbox3D::RenderMapPanel(const ImGuiViewport* viewport, float topPanelWidth, float minTopPanelHeight, float maxTopPanelHeight)
{
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(topPanelWidth, m_TopMapPanelHeight), ImGuiCond_Always);
	ImGuiWindowFlags topMapFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Map", nullptr, topMapFlags);
	m_TopMapPanelHeight = ImGui::GetWindowSize().y;
	if (m_TopMapPanelHeight < minTopPanelHeight)
	{
		m_TopMapPanelHeight = minTopPanelHeight;
	}
	if (m_TopMapPanelHeight > maxTopPanelHeight)
	{
		m_TopMapPanelHeight = maxTopPanelHeight;
	}

	ImGui::InputText("Map Path", m_MapPathBuffer, sizeof(m_MapPathBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Save Map"))
	{
		SaveMapToFile(m_MapPathBuffer);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Map"))
	{
		LoadMapFromFile(m_MapPathBuffer);
		if (m_IsPlaying)
		{
			EnsurePlaySessionActorPossession();
		}
	}
	ImGui::SameLine();
	if (!m_IsPlaying)
	{
		if (ImGui::Button("Play"))
		{
			StartPlayMode();
		}
	}
	else
	{
		if (ImGui::Button("Stop"))
		{
			StopPlayMode();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Create New Map"))
	{
		if (Achengine::WorldActorCache* cache = Achengine::WorldActorCache::Get())
		{
			cache->ClearActorCache();
			m_SelectedActors.clear();
			m_ActiveActor = nullptr;
			ResetActorTemplateEditor();
			m_ModelActor = nullptr;
			m_ModelMesh = nullptr;
			m_PlayerActor = nullptr;
			m_MapStatus = "Created new empty map";
			SpawnDefaultScene();
			if (m_IsPlaying)
			{
				EnsurePlaySessionActorPossession();
			}
		}
		else
		{
			m_MapStatus = "World cache is unavailable";
		}
	}
	ImGui::TextUnformatted(m_MapStatus.c_str());
	ImGui::Separator();
	ImGui::TextUnformatted("Renderer Stats");
	const Achengine::FRendererStats rendererStats = Achengine::Renderer::GetStats();
	ImGui::Text("Draw Calls: %u", rendererStats.DrawCalls);
	ImGui::Text("Meshes Queued: %u", rendererStats.MeshesQueued);
	ImGui::Text("Mesh Batches: %u", rendererStats.MeshBatches);
	ImGui::End();
}

void Sandbox3D::OnImGuiRender()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float minOutlinerWidth = 220.0f;
	const float maxOutlinerWidth = viewport->WorkSize.x * 0.65f;
	if (m_OutlinerWidth < minOutlinerWidth)
	{
		m_OutlinerWidth = minOutlinerWidth;
	}
	if (m_OutlinerWidth > maxOutlinerWidth)
	{
		m_OutlinerWidth = maxOutlinerWidth;
	}

	const float usableAssetWidth = viewport->WorkSize.x - m_OutlinerWidth;
	const float minTopPanelHeight = 70.0f;
	float maxTopPanelHeight = viewport->WorkSize.y - m_AssetBrowserHeight - 80.0f;
	if (maxTopPanelHeight < minTopPanelHeight)
	{
		maxTopPanelHeight = minTopPanelHeight;
	}
	if (m_TopMapPanelHeight < minTopPanelHeight)
	{
		m_TopMapPanelHeight = minTopPanelHeight;
	}
	if (m_TopMapPanelHeight > maxTopPanelHeight)
	{
		m_TopMapPanelHeight = maxTopPanelHeight;
	}

	const float minPanelHeight = 140.0f;
	float maxPanelHeight = viewport->WorkSize.y - m_TopMapPanelHeight - 80.0f;
	if (maxPanelHeight < minPanelHeight)
	{
		maxPanelHeight = minPanelHeight;
	}
	if (m_AssetBrowserHeight < minPanelHeight)
	{
		m_AssetBrowserHeight = minPanelHeight;
	}
	if (m_AssetBrowserHeight > maxPanelHeight)
	{
		m_AssetBrowserHeight = maxPanelHeight;
	}

	RenderOutlinerPanel(viewport, minOutlinerWidth, maxOutlinerWidth);
	const float topPanelWidth = viewport->WorkSize.x - m_OutlinerWidth;
	RenderMapPanel(viewport, topPanelWidth, minTopPanelHeight, maxTopPanelHeight);

	const float assetWidth = viewport->WorkSize.x - m_OutlinerWidth;
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - m_AssetBrowserHeight), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(assetWidth, m_AssetBrowserHeight), ImGuiCond_Always);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Asset browser", nullptr, windowFlags);
	m_AssetBrowserHeight = ImGui::GetWindowSize().y;
	if (m_AssetBrowserHeight < minPanelHeight)
	{
		m_AssetBrowserHeight = minPanelHeight;
	}
	if (m_AssetBrowserHeight > maxPanelHeight)
	{
		m_AssetBrowserHeight = maxPanelHeight;
	}

	static std::string modelStatus = "";
	static bool browserInitialized = false;
	static std::string browserDirectory;
	static const std::string payloadType = "FILE_PATH_PAYLOAD";

	auto LoadModelAtPath = [&](const std::string& modelPath, const glm::vec3* spawnWorldLocation = nullptr, bool forceCreateNewActor = false) {
		if (!FileExists(modelPath))
		{
			modelStatus = "Model file not found";
			return;
		}

		if (forceCreateNewActor || !m_ModelActor)
		{
			Achengine::AActor* newActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
			newActor->SetActorName("ImportedModel");
			Achengine::UMesh* newMesh = new Achengine::UMesh(modelPath);
			newActor->AddActorComponent(newMesh);
			if (spawnWorldLocation)
			{
				newActor->SetActorLocation(*spawnWorldLocation);
			}
			else
			{
				newActor->SetActorLocation({0.0f, 30.0f, 0.0f});
			}
			newActor->SetActorScale(m_ModelScale);

			m_ModelActor = newActor;
			m_ModelMesh = newMesh;
			modelStatus = forceCreateNewActor ? "Model actor created from drop" : "Model actor created";
			return;
		}

		if (m_ModelMesh && m_ModelMesh->ReloadModel(modelPath))
		{
			if (spawnWorldLocation && m_ModelActor)
			{
				m_ModelActor->SetActorLocation(*spawnWorldLocation);
			}
			std::strncpy(m_ModelPathBuffer, modelPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
			m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
			modelStatus = "Model reloaded";
		}
		else
		{
			modelStatus = "Model reload failed (see logs)";
		}
	};

	if (!browserInitialized)
	{
		browserDirectory = GetDirectoryFromPath(m_ModelPathBuffer);
		browserInitialized = true;
	}

	ImGui::InputText("Model Path", m_ModelPathBuffer, sizeof(m_ModelPathBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Load / Reload Model"))
	{
		LoadModelAtPath(m_ModelPathBuffer);
	}
	ImGui::SameLine();
	ImGui::Button("Drop File Here", ImVec2(140.0f, 0.0f));
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType.c_str()))
		{
			const char* droppedPath = (const char*)payload->Data;
			if (droppedPath)
			{
				std::string dropped = droppedPath;
				if (IsModelFile(dropped))
				{
					std::strncpy(m_ModelPathBuffer, dropped.c_str(), sizeof(m_ModelPathBuffer) - 1);
					m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
					LoadModelAtPath(dropped, nullptr, true);
				}
				else if (IsJsonFile(dropped))
				{
					std::strncpy(m_MapPathBuffer, dropped.c_str(), sizeof(m_MapPathBuffer) - 1);
					m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';
					m_MapStatus = "Dropped JSON file. Click Load Map to load.";
				}
				else
				{
					modelStatus = "Dropped file is not a model or map JSON";
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(modelStatus.c_str());

	ImGui::Separator();
	ImGui::TextUnformatted("File Browser");
	ImGui::SameLine();
	if (ImGui::Button("Up"))
	{
		const size_t slash = browserDirectory.find_last_of("/\\");
		if (slash != std::string::npos && slash > 0)
		{
			browserDirectory = browserDirectory.substr(0, slash);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Here"))
	{
		browserDirectory = GetDirectoryFromPath(m_ModelPathBuffer);
	}

	ImGui::TextUnformatted("Path:");
	std::vector<std::string> pathSegments;
	std::string currentSegment;
	for (char c : browserDirectory)
	{
		if (c == '/' || c == '\\')
		{
			if (!currentSegment.empty())
			{
				pathSegments.push_back(currentSegment);
				currentSegment.clear();
			}
		}
		else
		{
			currentSegment.push_back(c);
		}
	}
	if (!currentSegment.empty())
	{
		pathSegments.push_back(currentSegment);
	}

	std::string crumbPath;
	if (!browserDirectory.empty() && (browserDirectory[0] == '/' || browserDirectory[0] == '\\'))
	{
		crumbPath = "/";
		ImGui::SameLine();
		if (ImGui::SmallButton("/"))
		{
			browserDirectory = "/";
		}
		if (!pathSegments.empty())
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(">");
		}
	}

	for (size_t i = 0; i < pathSegments.size(); ++i)
	{
		if (!crumbPath.empty() && crumbPath.back() != '/')
		{
			crumbPath += '/';
		}
		crumbPath += pathSegments[i];

		ImGui::SameLine();
		ImGui::PushID((int)i + 9000);
		if (ImGui::SmallButton(pathSegments[i].c_str()))
		{
			browserDirectory = crumbPath;
		}
		ImGui::PopID();
		if (i + 1 < pathSegments.size())
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(">");
		}
	}

	ImGui::BeginChild("AssetFileBrowser", ImVec2(0.0f, 170.0f), true);
	std::vector<FBrowserEntry> entries = ReadDirectoryEntries(browserDirectory);
	if (entries.empty())
	{
		ImGui::TextUnformatted("No asset files/folders found here.");
	}
	for (const FBrowserEntry& entry : entries)
	{
		const bool isModel = !entry.IsDirectory && IsModelFile(entry.Name);
		const bool isTexture = !entry.IsDirectory && IsTextureFile(entry.Name);
		const bool isJson = !entry.IsDirectory && IsJsonFile(entry.Name);

		std::string displayName;
		if (entry.IsDirectory)
		{
			displayName = "[DIR] " + entry.Name;
		}
		else if (isModel)
		{
			displayName = "[MODEL] " + entry.Name;
		}
		else if (isTexture)
		{
			displayName = "[TEX] " + entry.Name;
		}
		else if (isJson)
		{
			displayName = "[JSON] " + entry.Name;
		}
		else
		{
			displayName = entry.Name;
		}

		ImVec4 itemColor = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
		if (entry.IsDirectory)
		{
			itemColor = ImVec4(0.95f, 0.82f, 0.35f, 1.0f);
		}
		else if (isModel)
		{
			itemColor = ImVec4(0.35f, 0.78f, 0.98f, 1.0f);
		}
		else if (isTexture)
		{
			itemColor = ImVec4(0.45f, 0.92f, 0.55f, 1.0f);
		}
		else if (isJson)
		{
			itemColor = ImVec4(0.95f, 0.55f, 0.35f, 1.0f);
		}

		ImGui::PushID(entry.FullPath.c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, itemColor);
		if (ImGui::Selectable(displayName.c_str()))
		{
			if (entry.IsDirectory)
			{
				browserDirectory = entry.FullPath;
			}
			else
			{
				if (isModel)
				{
					std::strncpy(m_ModelPathBuffer, entry.FullPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
					m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
				}
				if (isJson)
				{
					const EJsonAssetType jsonType = ClassifyJsonAssetFile(entry.FullPath);
					if (jsonType == EJsonAssetType::ActorTemplate)
					{
						std::string templateEditorError;
						if (LoadActorTemplateEditorFromFile(entry.FullPath, templateEditorError))
						{
							m_SelectedActors.clear();
							m_ActiveActor = nullptr;
						}
						else
						{
							ResetActorTemplateEditor();
							m_MapStatus = Achengine::format("Template load failed: %s", templateEditorError.c_str());
						}
					}
					else if (jsonType == EJsonAssetType::Map)
					{
						ResetActorTemplateEditor();
						std::strncpy(m_MapPathBuffer, entry.FullPath.c_str(), sizeof(m_MapPathBuffer) - 1);
						m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';
						m_MapStatus = "Map selected";
					}
					else
					{
						ResetActorTemplateEditor();
						m_MapStatus = "JSON is not a map or actor template";
					}
				}
			}
		}
		if (!entry.IsDirectory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && isModel)
		{
			LoadModelAtPath(entry.FullPath);
		}
		if (!entry.IsDirectory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && isJson)
		{
			const EJsonAssetType jsonType = ClassifyJsonAssetFile(entry.FullPath);
			if (jsonType == EJsonAssetType::ActorTemplate)
			{
				std::string templateEditorError;
				if (LoadActorTemplateEditorFromFile(entry.FullPath, templateEditorError))
				{
					m_SelectedActors.clear();
					m_ActiveActor = nullptr;
				}
				else
				{
					ResetActorTemplateEditor();
					m_MapStatus = Achengine::format("Template load failed: %s", templateEditorError.c_str());
				}
			}
			else if (jsonType == EJsonAssetType::Map)
			{
				ResetActorTemplateEditor();
				std::strncpy(m_MapPathBuffer, entry.FullPath.c_str(), sizeof(m_MapPathBuffer) - 1);
				m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';
				LoadMapFromFile(entry.FullPath);
				if (m_IsPlaying)
				{
					EnsurePlaySessionActorPossession();
				}
			}
			else
			{
				m_MapStatus = "JSON is not a map or actor template";
			}
		}
		if (!entry.IsDirectory && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(payloadType.c_str(), entry.FullPath.c_str(), entry.FullPath.size() + 1);
			ImGui::Text("%s", displayName.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleColor();
		ImGui::PopID();
	}

	if (ImGui::BeginPopupContextWindow("AssetBrowserEmptyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
	{
		static bool requestOpenCreateActorPopup = false;
		if (ImGui::Button("Create Actor"))
		{
			requestOpenCreateActorPopup = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();

		if (requestOpenCreateActorPopup)
		{
			ImGui::OpenPopup("CreateActorConfigPopup");
			requestOpenCreateActorPopup = false;
		}
	}

	struct FComponentChoice
	{
		const char* ClassName;
		const char* TemplateType;
		bool Selectable;
	};

	static const FComponentChoice componentChoices[] = {
		{"UActorComponent (base)", "actorComponent", false},
		{"UMesh", "mesh", true},
		{"UWaterMesh", "water", true},
		{"ULightComponent", "light", true},
		{"UCameraComponent", "camera", true}
	};

	static char createActorTemplateName[128] = "NewActorTemplate";
	static char createActorFileName[128] = "NewActorTemplate.json";
	static char createActorMeshModelPath[512] = "../models/Cube.fbx";
	static std::vector<std::string> selectedComponentTypes;
	static std::string createActorStatus;
	static bool meshPickerInitialized = false;
	static std::string meshPickerDirectory;
	static float createActorLightColor[3] = {1.0f, 0.95f, 0.85f};
	static float createActorLightAmbient[3] = {0.15f, 0.14f, 0.12f};
	static float createActorLightDiffuse[3] = {0.9f, 0.8f, 0.7f};
	static float createActorLightSpecular[3] = {1.0f, 0.95f, 0.9f};
	static float createActorLightConstant = 1.0f;
	static float createActorLightLinear = 0.022f;
	static float createActorLightQuadratic = 0.0019f;
	static float createActorCameraRelativeLocation[3] = {-15.0f, 0.0f, 0.0f};
	static float createActorCameraRelativeRotationAxis[3] = {1.0f, 0.0f, 0.0f};
	static float createActorCameraRelativeRotationAngle = 0.0f;
	static float createActorCameraFov = 90.0f;
	static float createActorCameraNearClip = 0.1f;
	static float createActorCameraFarClip = 1000.0f;

	if (ImGui::BeginPopupModal("CreateActorConfigPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Create Actor Template");
		ImGui::Separator();
		ImGui::InputText("Template Name", createActorTemplateName, sizeof(createActorTemplateName));
		ImGui::InputText("File Name", createActorFileName, sizeof(createActorFileName));

		if (ImGui::Button("Add component"))
		{
			ImGui::OpenPopup("AddComponentScrollPopup");
		}
		ImGui::SameLine();
		ImGui::Text("Selected: %d", (int)selectedComponentTypes.size());

		if (ImGui::BeginPopup("AddComponentScrollPopup"))
		{
			ImGui::TextUnformatted("ActorComponent classes");
			ImGui::Separator();
			ImGui::BeginChild("ComponentScrollBox", ImVec2(320.0f, 180.0f), true);
			for (const FComponentChoice& choice : componentChoices)
			{
				if (!choice.Selectable)
				{
					ImGui::TextDisabled("%s", choice.ClassName);
					continue;
				}

				const bool isSelected = std::find(selectedComponentTypes.begin(), selectedComponentTypes.end(), choice.TemplateType) != selectedComponentTypes.end();
				const std::string label = std::string(choice.ClassName) + " (" + choice.TemplateType + ")";
				if (ImGui::Selectable(label.c_str(), isSelected))
				{
					if (isSelected)
					{
						auto it = std::remove(selectedComponentTypes.begin(), selectedComponentTypes.end(), choice.TemplateType);
						selectedComponentTypes.erase(it, selectedComponentTypes.end());
					}
					else
					{
						selectedComponentTypes.push_back(choice.TemplateType);
					}
				}
			}
			ImGui::EndChild();
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (!selectedComponentTypes.empty())
		{
			ImGui::Separator();
			ImGui::TextUnformatted("Configured components:");
			int removeComponentIndex = -1;
			for (size_t i = 0; i < selectedComponentTypes.size(); ++i)
			{
				const std::string& componentType = selectedComponentTypes[i];
				ImGui::PushID((int)i + 50000);
				ImGui::BulletText("%s", componentType.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove"))
				{
					removeComponentIndex = (int)i;
				}
				ImGui::PopID();
			}

			if (removeComponentIndex >= 0)
			{
				selectedComponentTypes.erase(selectedComponentTypes.begin() + removeComponentIndex);
			}

			if (ImGui::Button("Clear All Components"))
			{
				selectedComponentTypes.clear();
			}
		}

		const bool hasMesh = std::find(selectedComponentTypes.begin(), selectedComponentTypes.end(), "mesh") != selectedComponentTypes.end();
		const bool hasLight = std::find(selectedComponentTypes.begin(), selectedComponentTypes.end(), "light") != selectedComponentTypes.end();
		const bool hasCamera = std::find(selectedComponentTypes.begin(), selectedComponentTypes.end(), "camera") != selectedComponentTypes.end();

		if (hasMesh || hasLight || hasCamera)
		{
			ImGui::Separator();
			ImGui::TextUnformatted("Component properties:");

			if (hasMesh)
			{
				ImGui::TextUnformatted("mesh");
				ImGui::InputText("Mesh Model Path", createActorMeshModelPath, sizeof(createActorMeshModelPath));
				ImGui::SameLine();
				if (ImGui::Button("Pick..."))
				{
					if (!meshPickerInitialized)
					{
						meshPickerDirectory = browserDirectory.empty() ? GetDirectoryFromPath(m_ModelPathBuffer) : browserDirectory;
						meshPickerInitialized = true;
					}
					ImGui::OpenPopup("PickMeshModelPathPopup");
				}

				if (ImGui::BeginPopup("PickMeshModelPathPopup"))
				{
					ImGui::TextUnformatted("Pick Mesh Model File");
					ImGui::Separator();

					if (ImGui::Button("Up"))
					{
						const size_t slash = meshPickerDirectory.find_last_of("/\\");
						if (slash != std::string::npos && slash > 0)
						{
							meshPickerDirectory = meshPickerDirectory.substr(0, slash);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Here"))
					{
						meshPickerDirectory = browserDirectory.empty() ? GetDirectoryFromPath(m_ModelPathBuffer) : browserDirectory;
					}
					ImGui::SameLine();
					ImGui::TextWrapped("%s", meshPickerDirectory.c_str());

					ImGui::BeginChild("MeshPickerFileList", ImVec2(420.0f, 220.0f), true);
					std::vector<FBrowserEntry> pickerEntries = ReadDirectoryEntries(meshPickerDirectory);
					if (pickerEntries.empty())
					{
						ImGui::TextUnformatted("No model files/folders found here.");
					}

					for (const FBrowserEntry& pickerEntry : pickerEntries)
					{
						if (!pickerEntry.IsDirectory && !IsModelFile(pickerEntry.Name))
						{
							continue;
						}

						const std::string pickerLabel = pickerEntry.IsDirectory
							? ("[DIR] " + pickerEntry.Name)
							: ("[MODEL] " + pickerEntry.Name);

						if (ImGui::Selectable(pickerLabel.c_str(), false))
						{
							if (pickerEntry.IsDirectory)
							{
								meshPickerDirectory = pickerEntry.FullPath;
							}
							else
							{
								std::strncpy(createActorMeshModelPath, pickerEntry.FullPath.c_str(), sizeof(createActorMeshModelPath) - 1);
								createActorMeshModelPath[sizeof(createActorMeshModelPath) - 1] = '\0';
								ImGui::CloseCurrentPopup();
							}
						}
					}
					ImGui::EndChild();

					if (ImGui::Button("Close"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}

			if (hasLight)
			{
				ImGui::Spacing();
				ImGui::TextUnformatted("light");
				ImGui::ColorEdit3("Light Color", createActorLightColor);
				ImGui::ColorEdit3("Light Ambient", createActorLightAmbient);
				ImGui::ColorEdit3("Light Diffuse", createActorLightDiffuse);
				ImGui::ColorEdit3("Light Specular", createActorLightSpecular);
				ImGui::InputFloat("Light Constant", &createActorLightConstant);
				ImGui::InputFloat("Light Linear", &createActorLightLinear);
				ImGui::InputFloat("Light Quadratic", &createActorLightQuadratic);
			}

			if (hasCamera)
			{
				ImGui::Spacing();
				ImGui::TextUnformatted("camera");
				ImGui::InputFloat3("Camera Relative Location", createActorCameraRelativeLocation);
				ImGui::InputFloat3("Camera Rotation Axis", createActorCameraRelativeRotationAxis);
				ImGui::InputFloat("Camera Rotation Angle", &createActorCameraRelativeRotationAngle);
				ImGui::InputFloat("Camera FOV", &createActorCameraFov);
				ImGui::InputFloat("Camera Near Clip", &createActorCameraNearClip);
				ImGui::InputFloat("Camera Far Clip", &createActorCameraFarClip);
			}
		}

		ImGui::Separator();
		if (ImGui::Button("Confirm"))
		{
			createActorStatus.clear();

			std::string fileName = createActorFileName;
			if (fileName.empty())
			{
				fileName = std::string(createActorTemplateName) + ".json";
			}
			if (fileName.find('.') == std::string::npos)
			{
				fileName += ".json";
			}

			if (std::string(createActorTemplateName).empty())
			{
				createActorStatus = "Template name cannot be empty";
			}
			else if (selectedComponentTypes.empty())
			{
				createActorStatus = "Select at least one component";
			}
			else
			{
				std::string outPath = browserDirectory;
				if (outPath.empty())
				{
					outPath = ".";
				}
				if (outPath.back() != '/' && outPath.back() != '\\')
				{
					outPath += '/';
				}
				outPath += fileName;

				std::ostringstream actorJson;
				actorJson << "{\n";
				actorJson << "  \"templateName\": \"" << JsonEscape(createActorTemplateName) << "\",\n";
				actorJson << "  \"components\": [\n";

				for (size_t i = 0; i < selectedComponentTypes.size(); ++i)
				{
					const std::string& componentType = selectedComponentTypes[i];
					actorJson << "    {\n";
					actorJson << "      \"type\": \"" << JsonEscape(componentType) << "\"";

					if (componentType == "mesh")
					{
						actorJson << ",\n      \"modelPath\": \"" << JsonEscape(createActorMeshModelPath) << "\"\n";
					}
					else if (componentType == "light")
					{
						actorJson << ",\n      \"light\": {\n";
						actorJson << "        \"color\": [" << createActorLightColor[0] << ", " << createActorLightColor[1] << ", " << createActorLightColor[2] << "],\n";
						actorJson << "        \"ambient\": [" << createActorLightAmbient[0] << ", " << createActorLightAmbient[1] << ", " << createActorLightAmbient[2] << "],\n";
						actorJson << "        \"diffuse\": [" << createActorLightDiffuse[0] << ", " << createActorLightDiffuse[1] << ", " << createActorLightDiffuse[2] << "],\n";
						actorJson << "        \"specular\": [" << createActorLightSpecular[0] << ", " << createActorLightSpecular[1] << ", " << createActorLightSpecular[2] << "],\n";
						actorJson << "        \"constant\": " << createActorLightConstant << ",\n";
						actorJson << "        \"linear\": " << createActorLightLinear << ",\n";
						actorJson << "        \"quadratic\": " << createActorLightQuadratic << "\n";
						actorJson << "      }\n";
					}
					else if (componentType == "camera")
					{
						actorJson << ",\n      \"transform\": {\n";
						actorJson << "        \"location\": [" << createActorCameraRelativeLocation[0] << ", " << createActorCameraRelativeLocation[1] << ", " << createActorCameraRelativeLocation[2] << "],\n";
						actorJson << "        \"rotationAxis\": [" << createActorCameraRelativeRotationAxis[0] << ", " << createActorCameraRelativeRotationAxis[1] << ", " << createActorCameraRelativeRotationAxis[2] << "],\n";
						actorJson << "        \"rotationAngle\": " << createActorCameraRelativeRotationAngle << "\n";
						actorJson << "      },\n";
						actorJson << "      \"camera\": {\n";
						actorJson << "        \"fov\": " << createActorCameraFov << ",\n";
						actorJson << "        \"nearClip\": " << createActorCameraNearClip << ",\n";
						actorJson << "        \"farClip\": " << createActorCameraFarClip << "\n";
						actorJson << "      }\n";
					}
					else
					{
						actorJson << "\n";
					}

					actorJson << "    }";
					if (i + 1 < selectedComponentTypes.size())
					{
						actorJson << ",";
					}
					actorJson << "\n";
				}

				actorJson << "  ]\n";
				actorJson << "}\n";

				std::string writeError;
				if (WriteTextFileAtomically(outPath, actorJson.str(), writeError))
				{
					createActorStatus = "Actor template created";
					selectedComponentTypes.clear();
					ImGui::CloseCurrentPopup();
				}
				else
				{
					createActorStatus = Achengine::format("Create failed: %s", writeError.c_str());
				}
			}
		}

		if (!createActorStatus.empty())
		{
			ImGui::TextWrapped("%s", createActorStatus.c_str());
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	ImGui::EndChild();
	ImGui::End();

	const ImVec2 renderMin(viewport->WorkPos.x, viewport->WorkPos.y);
	const ImVec2 renderMax(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);
	const ImVec2 sceneMin(viewport->WorkPos.x, viewport->WorkPos.y + m_TopMapPanelHeight);
	const ImVec2 sceneMax(viewport->WorkPos.x + assetWidth, viewport->WorkPos.y + viewport->WorkSize.y - m_AssetBrowserHeight);
	const ImVec2 sceneSize(glm::max(1.0f, sceneMax.x - sceneMin.x), glm::max(1.0f, sceneMax.y - sceneMin.y));

	Achengine::Camera* activeCamera = GetActiveSceneCamera();
	if (!activeCamera)
	{
		return;
	}
	const glm::vec3 activeCameraPosition = GetActiveSceneCameraPosition(activeCamera);

	auto ComputeDropWorldLocation = [&](const ImVec2& mousePos) {
		const float width = sceneMax.x - sceneMin.x;
		const float height = sceneMax.y - sceneMin.y;
		if (width <= 1.0f || height <= 1.0f)
		{
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		const float x = ((mousePos.x - sceneMin.x) / width) * 2.0f - 1.0f;
		const float y = 1.0f - ((mousePos.y - sceneMin.y) / height) * 2.0f;
		const glm::mat4 viewProjection = activeCamera->GetViewProjection() * activeCamera->GetViewMatrix();
		const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

		glm::vec4 nearClip = glm::vec4(x, y, -1.0f, 1.0f);
		glm::vec4 farClip = glm::vec4(x, y, 1.0f, 1.0f);
		glm::vec4 nearWorld4 = inverseViewProjection * nearClip;
		glm::vec4 farWorld4 = inverseViewProjection * farClip;
		if (fabsf(nearWorld4.w) < 0.00001f || fabsf(farWorld4.w) < 0.00001f)
		{
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		const glm::vec3 nearWorld = glm::vec3(nearWorld4) / nearWorld4.w;
		const glm::vec3 farWorld = glm::vec3(farWorld4) / farWorld4.w;
		const glm::vec3 rayOrigin = activeCameraPosition;
		const glm::vec3 rayDir = glm::normalize(farWorld - nearWorld);

		const float epsilon = 0.0001f;
		if (fabsf(rayDir.y) < epsilon)
		{
			return nearWorld + rayDir * 20.0f;
		}

		const float t = -rayOrigin.y / rayDir.y;
		if (t <= 0.0f)
		{
			return nearWorld + rayDir * 20.0f;
		}

		return rayOrigin + rayDir * t;
	};

	// Add a dedicated drag-drop target over the scene viewport while dragging assets.
	if (const ImGuiPayload* activePayload = ImGui::GetDragDropPayload())
	{
		if (activePayload->IsDataType(payloadType.c_str()))
		{
			ImGui::SetNextWindowPos(sceneMin, ImGuiCond_Always);
			ImGui::SetNextWindowSize(sceneSize, ImGuiCond_Always);
			ImGuiWindowFlags sceneDropFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowBgAlpha(0.08f);
			ImGui::Begin("SceneDropTarget", nullptr, sceneDropFlags);
			ImGui::Dummy(sceneSize);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType.c_str()))
				{
					const char* droppedPath = (const char*)payload->Data;
					if (droppedPath)
					{
						const std::string dropped = droppedPath;
						if (IsModelFile(dropped))
						{
							std::strncpy(m_ModelPathBuffer, dropped.c_str(), sizeof(m_ModelPathBuffer) - 1);
							m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
							const glm::vec3 dropLocation = ComputeDropWorldLocation(ImGui::GetIO().MousePos);
							LoadModelAtPath(dropped, &dropLocation, true);
						}
						else if (IsJsonFile(dropped))
						{
							const glm::vec3 dropLocation = ComputeDropWorldLocation(ImGui::GetIO().MousePos);
							std::string templateSpawnStatus;
							const std::string templateType = NormalizeActorTemplatePathForMap(m_MapPathBuffer, dropped);
							Achengine::AActor* droppedTemplateActor = nullptr;
							const EActorTemplateLoadResult templateResult = SpawnActorTemplateFromJson(dropped, dropLocation, templateSpawnStatus, &droppedTemplateActor, templateType);
							if (templateResult == EActorTemplateLoadResult::Spawned)
							{
								if (droppedTemplateActor)
								{
									m_SelectedActors.clear();
									m_SelectedActors.insert(droppedTemplateActor);
									m_ActiveActor = droppedTemplateActor;
								}
								modelStatus = templateSpawnStatus;
							}
							else if (templateResult == EActorTemplateLoadResult::NotTemplate)
							{
								std::strncpy(m_MapPathBuffer, dropped.c_str(), sizeof(m_MapPathBuffer) - 1);
								m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';
								LoadMapFromFile(dropped);
								if (m_IsPlaying)
								{
									EnsurePlaySessionActorPossession();
								}
							}
							else
							{
								modelStatus = templateSpawnStatus;
							}
						}
						else
						{
							modelStatus = "Dropped file is not a model or map JSON";
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	auto IsPointInRect = [](const ImVec2& p, const ImVec2& min, const ImVec2& max) {
		return p.x >= min.x && p.y >= min.y && p.x <= max.x && p.y <= max.y;
	};

	auto DistancePointToSegment = [](const ImVec2& p, const ImVec2& a, const ImVec2& b) {
		const float vx = b.x - a.x;
		const float vy = b.y - a.y;
		const float wx = p.x - a.x;
		const float wy = p.y - a.y;
		const float c1 = vx * wx + vy * wy;
		if (c1 <= 0.0f)
		{
			const float dx = p.x - a.x;
			const float dy = p.y - a.y;
			return sqrtf(dx * dx + dy * dy);
		}

		const float c2 = vx * vx + vy * vy;
		if (c2 <= c1)
		{
			const float dx = p.x - b.x;
			const float dy = p.y - b.y;
			return sqrtf(dx * dx + dy * dy);
		}

		const float t = c1 / c2;
		const float projx = a.x + t * vx;
		const float projy = a.y + t * vy;
		const float dx = p.x - projx;
		const float dy = p.y - projy;
		return sqrtf(dx * dx + dy * dy);
	};

	auto WorldToScreen = [&](const glm::vec3& world, ImVec2& out) {
		glm::mat4 viewProjection = activeCamera->GetViewProjection() * activeCamera->GetViewMatrix();
		glm::vec4 clip = viewProjection * glm::vec4(world, 1.0f);
		if (clip.w <= 0.0001f)
		{
			return false;
		}

		glm::vec3 ndc = glm::vec3(clip) / clip.w;
		const float width = sceneMax.x - sceneMin.x;
		const float height = sceneMax.y - sceneMin.y;
		out.x = sceneMin.x + (ndc.x * 0.5f + 0.5f) * width;
		out.y = sceneMin.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * height;
		return true;
	};

	auto SnapDelta = [&](float delta, float step) {
		if (!m_UseGizmoSnapping || step <= 0.0f)
		{
			return delta;
		}

		return roundf(delta / step) * step;
	};

	auto IntersectRaySphere = [](const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter, float sphereRadius, float& outT) {
		const glm::vec3 oc = rayOrigin - sphereCenter;
		const float a = glm::dot(rayDir, rayDir);
		const float b = 2.0f * glm::dot(oc, rayDir);
		const float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
		const float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			return false;
		}

		const float sqrtDisc = sqrtf(discriminant);
		const float t0 = (-b - sqrtDisc) / (2.0f * a);
		const float t1 = (-b + sqrtDisc) / (2.0f * a);
		if (t0 > 0.0f)
		{
			outT = t0;
			return true;
		}
		if (t1 > 0.0f)
		{
			outT = t1;
			return true;
		}

		return false;
	};

	auto IntersectRayAABB = [](const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& minBounds, const glm::vec3& maxBounds, float& outT) {
		float tMin = 0.0f;
		float tMax = 10000000.0f;
		for (int i = 0; i < 3; ++i)
		{
			if (fabsf(rayDir[i]) < 0.000001f)
			{
				if (rayOrigin[i] < minBounds[i] || rayOrigin[i] > maxBounds[i])
				{
					return false;
				}
				continue;
			}

			const float invDir = 1.0f / rayDir[i];
			float t1 = (minBounds[i] - rayOrigin[i]) * invDir;
			float t2 = (maxBounds[i] - rayOrigin[i]) * invDir;
			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = glm::max(tMin, t1);
			tMax = glm::min(tMax, t2);
			if (tMin > tMax)
			{
				return false;
			}
		}

		outT = tMin > 0.0f ? tMin : tMax;
		return outT >= 0.0f;
	};

	const glm::mat4 viewProjection = activeCamera->GetViewProjection() * activeCamera->GetViewMatrix();
	const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
	const glm::vec3 canonicalAxes[3] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	};

	bool hasGizmo = false;
	ImVec2 originScreen = ImVec2(0.0f, 0.0f);
	ImVec2 endScreen[3] = { ImVec2(0.0f, 0.0f), ImVec2(0.0f, 0.0f), ImVec2(0.0f, 0.0f) };
	bool axisVisible[3] = { false, false, false };
	glm::vec3 axisDirectionsWorld[3] = { canonicalAxes[0], canonicalAxes[1], canonicalAxes[2] };
	glm::vec3 actorPos(0.0f);
	float gizmoWorldSize = 1.0f;

	if (!m_IsPlaying && m_ActiveActor && !m_SelectedActors.empty())
	{
		// Keep the gizmo centered on the selected actor bounds every frame.
		const Achengine::FBounds activeBounds = m_ActiveActor->GetBounds();
		actorPos = activeBounds.IsValid ? activeBounds.Center : m_ActiveActor->GetActorLocation();
		gizmoWorldSize = glm::max(1.0f, glm::distance(activeCameraPosition, actorPos) * 0.2f);

		Achengine::FRotation actorRot = m_ActiveActor->GetActorRotation();
		glm::vec3 axis = actorRot.RotationAxis;
		if (glm::length(axis) < 0.0001f)
		{
			axis = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		const glm::quat actorQuat = glm::angleAxis(glm::radians(actorRot.Angle), glm::normalize(axis));

		for (int i = 0; i < 3; ++i)
		{
			axisDirectionsWorld[i] = canonicalAxes[i];
			if (m_GizmoSpace == EGizmoSpace::Local)
			{
				axisDirectionsWorld[i] = glm::normalize(glm::rotate(actorQuat, canonicalAxes[i]));
			}
		}

		glm::vec3 axisWorldPoints[3] = {
			actorPos + axisDirectionsWorld[0] * gizmoWorldSize,
			actorPos + axisDirectionsWorld[1] * gizmoWorldSize,
			actorPos + axisDirectionsWorld[2] * gizmoWorldSize
		};

		hasGizmo = WorldToScreen(actorPos, originScreen);
		if (hasGizmo)
		{
			ImDrawList* drawList = ImGui::GetForegroundDrawList();
			const ImU32 axisColors[3] = {
				IM_COL32(245, 90, 90, 255),
				IM_COL32(90, 220, 120, 255),
				IM_COL32(90, 150, 245, 255)
			};

			for (int i = 0; i < 3; ++i)
			{
				if (WorldToScreen(axisWorldPoints[i], endScreen[i]))
				{
					axisVisible[i] = true;
					drawList->AddLine(originScreen, endScreen[i], axisColors[i], i == m_GizmoActiveAxis ? 4.0f : 2.5f);
					drawList->AddCircleFilled(endScreen[i], 4.0f, axisColors[i]);
				}
			}
		}
	}

	ImGuiIO& io = ImGui::GetIO();
	const ImVec2 mousePos = io.MousePos;
	const bool mouseInScene = IsPointInRect(mousePos, sceneMin, sceneMax);

	auto PickActorAtMouse = [&](const ImVec2& mouse) -> Achengine::AActor* {
		const float width = sceneMax.x - sceneMin.x;
		const float height = sceneMax.y - sceneMin.y;
		if (width <= 1.0f || height <= 1.0f)
		{
			return nullptr;
		}

		const float x = ((mouse.x - sceneMin.x) / width) * 2.0f - 1.0f;
		const float y = 1.0f - ((mouse.y - sceneMin.y) / height) * 2.0f;

		glm::vec4 nearClip = glm::vec4(x, y, -1.0f, 1.0f);
		glm::vec4 farClip = glm::vec4(x, y, 1.0f, 1.0f);
		glm::vec4 nearWorld4 = inverseViewProjection * nearClip;
		glm::vec4 farWorld4 = inverseViewProjection * farClip;
		if (fabsf(nearWorld4.w) < 0.00001f || fabsf(farWorld4.w) < 0.00001f)
		{
			return nullptr;
		}

		glm::vec3 nearWorld = glm::vec3(nearWorld4) / nearWorld4.w;
		glm::vec3 farWorld = glm::vec3(farWorld4) / farWorld4.w;
		glm::vec3 rayOrigin = activeCameraPosition;
		glm::vec3 rayDir = glm::normalize(farWorld - nearWorld);

		Achengine::AActor* bestActor = nullptr;
		float bestT = 10000000.0f;

		if (Achengine::WorldActorCache* cache = Achengine::WorldActorCache::Get())
		{
			for (Achengine::AActor* actor : cache->GetActorCache())
			{
				if (!actor)
				{
					continue;
				}

				Achengine::UMesh* ActorMesh = actor->GetComponentByClass<Achengine::UMesh>();
				if (!ActorMesh)
				{
					continue;
				}

				const Achengine::FBounds bounds = actor->GetBounds();
				if (!bounds.IsValid)
				{
					continue;
				}

				const float broadPhaseRadius = glm::max(0.1f, bounds.SphereRadius);
				float broadPhaseT = 0.0f;
				if (!IntersectRaySphere(rayOrigin, rayDir, bounds.Center, broadPhaseRadius, broadPhaseT))
				{
					continue;
				}

				const Achengine::FBounds meshBounds = ActorMesh->GetBounds();
				if (!meshBounds.IsValid)
				{
					continue;
				}

				const glm::mat4 actorTransform = actor->GetActorTransform();
				const glm::mat4 inverseActorTransform = glm::inverse(actorTransform);
				const glm::vec3 localRayOrigin = glm::vec3(inverseActorTransform * glm::vec4(rayOrigin, 1.0f));
				const glm::vec3 localRayDir = glm::vec3(inverseActorTransform * glm::vec4(rayDir, 0.0f));

				const glm::vec3 localMin = meshBounds.Center - meshBounds.Extents;
				const glm::vec3 localMax = meshBounds.Center + meshBounds.Extents;
				float localHitT = 0.0f;
				if (!IntersectRayAABB(localRayOrigin, localRayDir, localMin, localMax, localHitT))
				{
					continue;
				}

				const glm::vec3 localHitPoint = localRayOrigin + localRayDir * localHitT;
				const glm::vec3 worldHitPoint = glm::vec3(actorTransform * glm::vec4(localHitPoint, 1.0f));
				const float worldHitT = glm::dot(worldHitPoint - rayOrigin, rayDir);
				if (worldHitT < 0.0f)
				{
					continue;
				}

				if (worldHitT < bestT)
				{
					bestT = worldHitT;
					bestActor = actor;
				}
			}
		}

		return bestActor;
	};

	if (!m_IsPlaying && ImGui::IsMouseClicked(0) && mouseInScene && !io.WantCaptureMouse)
	{
		int clickedAxis = -1;
		if (hasGizmo)
		{
			float bestDistance = 12.0f;
			for (int axis = 0; axis < 3; ++axis)
			{
				if (!axisVisible[axis])
				{
					continue;
				}

				float dist = DistancePointToSegment(mousePos, originScreen, endScreen[axis]);
				if (dist < bestDistance)
				{
					bestDistance = dist;
					clickedAxis = axis;
				}
			}
		}

		if (clickedAxis >= 0)
		{
			m_GizmoActiveAxis = clickedAxis;
			m_GizmoDragging = true;
		}
		else
		{
			Achengine::AActor* picked = PickActorAtMouse(mousePos);
			if (picked)
			{
				if (io.KeyCtrl)
				{
					if (m_SelectedActors.count(picked) > 0)
					{
						m_SelectedActors.erase(picked);
						if (m_ActiveActor == picked)
						{
							m_ActiveActor = m_SelectedActors.empty() ? nullptr : *m_SelectedActors.begin();
						}
					}
					else
					{
						m_SelectedActors.insert(picked);
						m_ActiveActor = picked;
					}
				}
				else
				{
					m_SelectedActors.clear();
					m_SelectedActors.insert(picked);
					m_ActiveActor = picked;
				}
			}
			else if (!io.KeyCtrl)
			{
				m_SelectedActors.clear();
				m_ActiveActor = nullptr;
			}
		}
	}

	if (!m_IsPlaying && m_ActiveActor && !m_SelectedActors.empty())
	{
		if (!ImGui::IsMouseDown(0))
		{
			m_GizmoDragging = false;
			m_GizmoActiveAxis = -1;
		}

		if (m_GizmoDragging && m_GizmoActiveAxis >= 0 && hasGizmo && axisVisible[m_GizmoActiveAxis])
		{
			ImVec2 axisScreenDir(endScreen[m_GizmoActiveAxis].x - originScreen.x, endScreen[m_GizmoActiveAxis].y - originScreen.y);
			const float axisLen = sqrtf(axisScreenDir.x * axisScreenDir.x + axisScreenDir.y * axisScreenDir.y);
			if (axisLen > 0.0001f)
			{
				axisScreenDir.x /= axisLen;
				axisScreenDir.y /= axisLen;
				const ImVec2 mouseDelta = io.MouseDelta;
				const float dragAmountPixels = mouseDelta.x * axisScreenDir.x + mouseDelta.y * axisScreenDir.y;
				const float worldPerPixel = glm::max(0.0025f, glm::distance(activeCameraPosition, actorPos) * 0.0015f);

				for (Achengine::AActor* selectedActor : m_SelectedActors)
				{
					if (!selectedActor)
					{
						continue;
					}

					if (m_GizmoMode == EGizmoMode::Translate)
					{
						float moveDistance = SnapDelta(dragAmountPixels * worldPerPixel, m_TranslateSnapStep);
						selectedActor->SetActorLocation(selectedActor->GetActorLocation() + axisDirectionsWorld[m_GizmoActiveAxis] * moveDistance);
					}
					else if (m_GizmoMode == EGizmoMode::Scale)
					{
						glm::vec3 newScale = selectedActor->GetActorScale();
						float scaleDelta = SnapDelta(dragAmountPixels * worldPerPixel, m_ScaleSnapStep);
						newScale[m_GizmoActiveAxis] = glm::max(0.01f, newScale[m_GizmoActiveAxis] + scaleDelta);
						selectedActor->SetActorScale(newScale);
					}
					else if (m_GizmoMode == EGizmoMode::Rotate)
					{
						float rotateDeltaDeg = SnapDelta(dragAmountPixels * 0.45f, m_RotateSnapStep);
						Achengine::FRotation rot = selectedActor->GetActorRotation();
						glm::vec3 currentAxis = rot.RotationAxis;
						if (glm::length(currentAxis) < 0.0001f)
						{
							currentAxis = glm::vec3(1.0f, 0.0f, 0.0f);
						}

						const glm::quat currentQuat = glm::angleAxis(glm::radians(rot.Angle), glm::normalize(currentAxis));
						const glm::quat deltaQuat = glm::angleAxis(glm::radians(rotateDeltaDeg), canonicalAxes[m_GizmoActiveAxis]);
						const glm::quat newQuat = (m_GizmoSpace == EGizmoSpace::Local)
							? glm::normalize(currentQuat * deltaQuat)
							: glm::normalize(deltaQuat * currentQuat);

						glm::vec3 newAxis = glm::axis(newQuat);
						if (glm::length(newAxis) < 0.0001f)
						{
							newAxis = glm::vec3(1.0f, 0.0f, 0.0f);
						}
						selectedActor->SetActorRotation(glm::normalize(newAxis), glm::degrees(glm::angle(newQuat)));
					}
				}
			}
		}
	}
}

void Sandbox3D::OnEvent(Achengine::Event& event)
{
	if (m_IsPlaying)
	{
		Achengine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Achengine::KeyPressedEvent>([this](Achengine::KeyPressedEvent& keyEvent)
		{
			if (keyEvent.GetKeyCode() == ACHENGINE_KEY_ESCAPE)
			{
				StopPlayMode();
				return true;
			}

			return false;
		});

		if (event.IsHandled())
		{
			return;
		}

		if (!m_PlayerController || !m_PlayerController->GetPossessedPlayer())
		{
			EnsurePlaySessionActorPossession();
		}

		if (m_PlayerController)
		{
			m_PlayerController->OnEvent(event);
		}
	}
	else if (!event.IsHandled())
	{
		Achengine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Achengine::KeyPressedEvent>([this](Achengine::KeyPressedEvent& keyEvent)
		{
			if (keyEvent.GetKeyCode() == ACHENGINE_KEY_ESCAPE)
			{
				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureKeyboard || m_SelectedActors.empty())
				{
					return false;
				}

				m_SelectedActors.clear();
				m_ActiveActor = nullptr;
				m_MapStatus = "Selection cleared";
				return true;
			}

			if (keyEvent.GetKeyCode() != ACHENGINE_KEY_DELETE)
			{
				return false;
			}

			ImGuiIO& io = ImGui::GetIO();
			if (io.WantCaptureKeyboard || !m_ActiveActor || m_SelectedActors.empty())
			{
				return false;
			}

			std::vector<Achengine::AActor*> actorsToDelete;
			actorsToDelete.reserve(m_SelectedActors.size());
			for (Achengine::AActor* actor : m_SelectedActors)
			{
				if (actor)
				{
					actorsToDelete.push_back(actor);
				}
			}

			for (Achengine::AActor* actor : actorsToDelete)
			{
				if (actor == m_ModelActor)
				{
					m_ModelActor = nullptr;
					m_ModelMesh = nullptr;
				}
				if (actor == m_PlayerActor)
				{
					m_PlayerActor = nullptr;
				}

				Achengine::WorldActorCache::DestroyActor(actor);
			}

			m_SelectedActors.clear();
			m_ActiveActor = nullptr;
			m_MapStatus = Achengine::format("Deleted %d actor(s)", (int)actorsToDelete.size());
			return !actorsToDelete.empty();
		});

		if (event.IsHandled())
		{
			return;
		}

		m_CameraController->OnEvent(event);
	}
}
