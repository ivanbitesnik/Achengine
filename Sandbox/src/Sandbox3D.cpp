#include "Sandbox3D.h"
// ----------------------------------------

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <random>
#include <unordered_map>
#include <vector>

#include "../../Achengine/vendor/Glad/include/glad/glad.h"

static glm::vec3 ToTransform(glm::vec3 vec)
{
	return glm::vec3(vec.y, vec.z, vec.x);
}

static bool FileExists(const std::string& path)
{
	std::ifstream stream(path);
	return stream.good();
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
	class FSandboxPlayerController : public Achengine::APlayerController
	{
	public:
		void Tick(float DeltaTime) override
		{
			Achengine::APlayer* player = GetPossessedPlayer();
			if (player)
			{
				glm::vec3 moveDirection(0.0f, 0.0f, 0.0f);
				if (IsKeyDown(ACHENGINE_KEY_W)) { moveDirection.z -= 1.0f; }
				if (IsKeyDown(ACHENGINE_KEY_S)) { moveDirection.z += 1.0f; }
				if (IsKeyDown(ACHENGINE_KEY_A)) { moveDirection.x -= 1.0f; }
				if (IsKeyDown(ACHENGINE_KEY_D)) { moveDirection.x += 1.0f; }
				if (IsKeyDown(ACHENGINE_KEY_Q)) { moveDirection.y += 1.0f; }
				if (IsKeyDown(ACHENGINE_KEY_E)) { moveDirection.y -= 1.0f; }

				if (glm::length(moveDirection) > 0.0001f)
				{
					moveDirection = glm::normalize(moveDirection);
					player->SetActorLocation(player->GetActorLocation() + moveDirection * m_MoveSpeed * DeltaTime);
				}

				if (Achengine::USpringArmComponent* springArm = player->GetSpringArm())
				{
					if (IsMouseButtonDown(ACHENGINE_MOUSE_BUTTON_RIGHT))
					{
						m_YawDegrees += GetMouseDelta().x * m_MouseLookSensitivity;
						m_PitchDegrees -= GetMouseDelta().y * m_MouseLookSensitivity;
						m_PitchDegrees = glm::clamp(m_PitchDegrees, -80.0f, 20.0f);
					}

					springArm->SetRelativeRotation(glm::vec3(1.0f, 0.0f, 0.0f), m_PitchDegrees);
					player->SetActorRotation(glm::vec3(0.0f, 1.0f, 0.0f), m_YawDegrees);
				}
			}

			APlayerController::Tick(DeltaTime);
		}

	private:
		float m_MoveSpeed = 10.0f;
		float m_MouseLookSensitivity = 0.1f;
		float m_YawDegrees = 0.0f;
		float m_PitchDegrees = -20.0f;
	};

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

	static EActorTemplateLoadResult SpawnActorTemplateFromJson(const std::string& filePath, const glm::vec3& dropLocation, std::string& outStatus)
	{
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
					actor->SetMesh(new Achengine::UMesh(modelPath));
				}
			}
			else if (componentType == "water")
			{
				actor->SetMesh(new Achengine::UWaterMesh());
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
		m_PlayerController = new FSandboxPlayerController();
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
		if (!playerStarts.empty())
		{
			static std::mt19937 rng(std::random_device{}());
			std::uniform_int_distribution<size_t> distribution(0, playerStarts.size() - 1);
			spawnLocation = playerStarts[distribution(rng)]->GetActorLocation();
		}

		m_PlayerActor->SetActorLocation(spawnLocation);
		m_PlayerActor->SetActorRotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
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
		m_PlayerController = new FSandboxPlayerController();
	}
	m_IsPlaying = true;
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
	m_MapStatus = "Play mode stopped";
}

void Sandbox3D::SpawnDefaultScene()
{
	Achengine::AActor* actor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	actor->SetActorName(Achengine::format("Floor"));
	actor->SetMesh(new Achengine::UMesh(GetDefaultCubeModelPath()));
	actor->SetActorLocation({0.0f, 0.0f, 0.0f});
	actor->SetActorScale({100.0f, 1.0f, 100.0f});

	Achengine::AActor* lightActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	lightActor->SetActorName(Achengine::format("Light"));
	lightActor->AddActorComponent(new Achengine::ULightComponent());
	lightActor->SetActorLocation({0.0f, 10.0f, 0.0f});
}

void Sandbox3D::OnDetach()
{
	StopPlayMode();
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
	Achengine::RenderCommand::SetClearColor({ 0.4f, 0.4f, 0.8f, 0.3f });
	Achengine::RenderCommand::Clear();

	if (m_IsPlaying && m_PlayerActor)
	{
		Achengine::Renderer::BeginScene(m_PlayerActor->GetCameraComponent());
	}
	else if (m_CameraController)
	{
		Achengine::Renderer::BeginScene(m_CameraController->GetCamera());
	}
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
}

bool Sandbox3D::SaveMapToFile(const std::string& filePath)
{
	if (filePath.empty())
	{
		m_MapStatus = "Map path is empty";
		return false;
	}

	std::ofstream out(filePath);
	if (!out.is_open())
	{
		m_MapStatus = "Failed to open map file for write";
		return false;
	}

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

			if (actor->GetMesh() || actor->GetComponentByClass<Achengine::ULightComponent>())
			{
				actors.push_back(actor);
			}
		}
	}

	std::sort(actors.begin(), actors.end());
	for (size_t i = 0; i < actors.size(); ++i)
	{
		Achengine::AActor* actor = actors[i];
		Achengine::UMesh* mesh = actor->GetMesh();
		Achengine::ULightComponent* lightComponent = actor->GetComponentByClass<Achengine::ULightComponent>();
		std::string meshType = "unknown";
		if (lightComponent)
		{
			meshType = "light";
		}
		else if (dynamic_cast<Achengine::UWaterMesh*>(mesh))
		{
			meshType = "water";
		}
		else
		{
			meshType = "mesh";
		}

		const glm::vec3 location = actor->GetActorLocation();
		const glm::vec3 scale = actor->GetActorScale();
		Achengine::FActorRotation rotation = actor->GetActorRotation();
		glm::vec3 rotAxis = rotation.RotationAxis;
		if (glm::length(rotAxis) < 0.0001f)
		{
			rotAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		out << "    {\n";
		out << "      \"name\": \"" << JsonEscape(actor->GetActorName()) << "\",\n";
		out << "      \"meshType\": \"" << meshType << "\",\n";
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
		else if (!mesh->GetModelPath().empty())
		{
			out << ",\n      \"modelPath\": \"" << JsonEscape(mesh->GetModelPath()) << "\"";
		}

		out << "\n    }";
		if (i + 1 < actors.size())
		{
			out << ",";
		}
		out << "\n";
	}

	out << "  ]\n";
	out << "}\n";
	out.close();

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
		std::string meshType;
		glm::vec3 location(0.0f);
		glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
		glm::vec3 scale(1.0f);
		float rotationAngle = 0.0f;
		JsonReadStringField(actorValue, "name", name);
		JsonReadStringField(actorValue, "meshType", meshType);
		JsonReadVec3Field(actorValue, "location", location);
		JsonReadVec3Field(actorValue, "rotationAxis", rotationAxis);
		JsonReadNumberField(actorValue, "rotationAngle", rotationAngle);
		JsonReadVec3Field(actorValue, "scale", scale);

		Achengine::UMesh* mesh = nullptr;
		Achengine::ULightComponent* lightComponent = nullptr;
		if (meshType == "light")
		{
			lightComponent = new Achengine::ULightComponent();
			FJsonValue lightValue;
			if (JsonReadObjectField(actorValue, "light", lightValue) && lightValue.Type == FJsonValue::EType::Object)
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
		}
		else if (meshType == "model" || meshType == "static" || meshType == "mesh")
		{
			std::string modelPath;
			JsonReadStringField(actorValue, "modelPath", modelPath);
			if (modelPath.empty() || !FileExists(modelPath))
			{
				modelPath = GetDefaultCubeModelPath();
				if (!FileExists(modelPath))
				{
					continue;
				}
			}

			mesh = new Achengine::UMesh(modelPath);
			std::strncpy(m_ModelPathBuffer, modelPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
			m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
		}
		else if (meshType == "water")
		{
			mesh = new Achengine::UWaterMesh();
		}
		else
		{
			std::string modelPath = GetDefaultCubeModelPath();
			if (FileExists(modelPath))
			{
				mesh = new Achengine::UMesh(modelPath);
				std::strncpy(m_ModelPathBuffer, modelPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
				m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
			}
		}

		if (!mesh && !lightComponent)
		{
			continue;
		}

		Achengine::AActor* actor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
		if (!name.empty())
		{
			actor->SetActorName(name);
		}

		if (mesh)
		{
			actor->SetMesh(mesh);
			if (!dynamic_cast<Achengine::UWaterMesh*>(mesh))
			{
				m_ModelActor = actor;
				m_ModelMesh = mesh;
			}
		}
		if (lightComponent)
		{
			actor->AddActorComponent(lightComponent);
		}
		actor->SetActorLocation(location);
		if (glm::length(rotationAxis) < 0.0001f)
		{
			rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		actor->SetActorRotation(glm::normalize(rotationAxis), rotationAngle);
		actor->SetActorScale(scale);
		++loadedActors;
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
	if (m_IsPlaying && m_PlayerActor && m_PlayerActor->GetCameraComponent())
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

	if (Achengine::WorldActorCache* Cache = Achengine::WorldActorCache::Get())
	{
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
		}
		if (!m_ActiveActor && !m_SelectedActors.empty())
		{
			m_ActiveActor = *m_SelectedActors.begin();
		}

		ImGui::Text("Actors: %d", (int)actors.size());
		ImGui::SameLine();
		ImGui::TextUnformatted("(Ctrl-click for multiselect)");
		ImGui::Separator();
		ImGui::BeginChild("OutlinerActors", ImVec2(0.0f, 220.0f), true);
		for (Achengine::AActor* actor : actors)
		{
			std::string shaderName = "<none>";
			if (Achengine::UMesh* mesh = actor->GetMesh())
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
						}
					}
					else
					{
						m_SelectedActors.insert(actor);
						m_ActiveActor = actor;
					}
				}
				else
				{
					m_SelectedActors.clear();
					m_SelectedActors.insert(actor);
					m_ActiveActor = actor;
				}
			}
			ImGui::PopID();
		}
		ImGui::EndChild();

		ImGui::Separator();
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

			Achengine::FActorRotation rotation = m_ActiveActor->GetActorRotation();
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
		}
		else
		{
			ImGui::TextUnformatted("No actor selected");
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
			newActor->SetMesh(newMesh);
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
					std::strncpy(m_MapPathBuffer, entry.FullPath.c_str(), sizeof(m_MapPathBuffer) - 1);
					m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';
				}
			}
		}
		if (!entry.IsDirectory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && isModel)
		{
			LoadModelAtPath(entry.FullPath);
		}
		if (!entry.IsDirectory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && isJson)
		{
			std::strncpy(m_MapPathBuffer, entry.FullPath.c_str(), sizeof(m_MapPathBuffer) - 1);
			m_MapPathBuffer[sizeof(m_MapPathBuffer) - 1] = '\0';
			LoadMapFromFile(entry.FullPath);
			if (m_IsPlaying)
			{
				EnsurePlaySessionActorPossession();
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
		const float width = renderMax.x - renderMin.x;
		const float height = renderMax.y - renderMin.y;
		if (width <= 1.0f || height <= 1.0f)
		{
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		const float x = ((mousePos.x - renderMin.x) / width) * 2.0f - 1.0f;
		const float y = 1.0f - ((mousePos.y - renderMin.y) / height) * 2.0f;
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
							const EActorTemplateLoadResult templateResult = SpawnActorTemplateFromJson(dropped, dropLocation, templateSpawnStatus);
							if (templateResult == EActorTemplateLoadResult::Spawned)
							{
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
		const float width = renderMax.x - renderMin.x;
		const float height = renderMax.y - renderMin.y;
		out.x = renderMin.x + (ndc.x * 0.5f + 0.5f) * width;
		out.y = renderMin.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * height;
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

	if (m_ActiveActor && !m_SelectedActors.empty())
	{
		// Keep the gizmo centered on the selected actor bounds every frame.
		const Achengine::FActorBounds activeBounds = m_ActiveActor->GetBounds();
		actorPos = activeBounds.IsValid ? activeBounds.Center : m_ActiveActor->GetActorLocation();
		gizmoWorldSize = glm::max(1.0f, glm::distance(activeCameraPosition, actorPos) * 0.2f);

		Achengine::FActorRotation actorRot = m_ActiveActor->GetActorRotation();
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
		const float width = renderMax.x - renderMin.x;
		const float height = renderMax.y - renderMin.y;
		if (width <= 1.0f || height <= 1.0f)
		{
			return nullptr;
		}

		const float x = ((mouse.x - renderMin.x) / width) * 2.0f - 1.0f;
		const float y = 1.0f - ((mouse.y - renderMin.y) / height) * 2.0f;

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
				if (!actor || !actor->GetMesh())
				{
					continue;
				}

				const Achengine::FActorBounds bounds = actor->GetBounds();
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

				const Achengine::FMeshBounds meshBounds = actor->GetMesh()->GetBounds();
				if (!meshBounds.IsValid)
				{
					continue;
				}

				const glm::mat4 actorTransform = actor->GetActorTransform();
				const glm::mat4 inverseActorTransform = glm::inverse(actorTransform);
				const glm::vec3 localRayOrigin = glm::vec3(inverseActorTransform * glm::vec4(rayOrigin, 1.0f));
				const glm::vec3 localRayDir = glm::vec3(inverseActorTransform * glm::vec4(rayDir, 0.0f));

				const glm::vec3 localMin = meshBounds.LocalCenter - meshBounds.LocalExtents;
				const glm::vec3 localMax = meshBounds.LocalCenter + meshBounds.LocalExtents;
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

	if (ImGui::IsMouseClicked(0) && mouseInScene && !io.WantCaptureMouse)
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

	if (m_ActiveActor && !m_SelectedActors.empty())
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
						Achengine::FActorRotation rot = selectedActor->GetActorRotation();
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
		m_CameraController->OnEvent(event);
	}
}
