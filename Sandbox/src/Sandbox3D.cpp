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
#include <sys/stat.h>
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
		if (name == ".")
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
		if (browserEntry.IsDirectory || IsModelFile(browserEntry.Name))
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

Sandbox3D::Sandbox3D()
	: Layer("Sandbox3D")
{
	m_CameraController = new Achengine::EditorCameraController(1280.0f / 720.0f);
}

Sandbox3D::~Sandbox3D()
{
	delete m_CameraController;
}

void Sandbox3D::OnAttach()
{
	m_Textures.insert({"Acheto", Achengine::Texture2D::Create("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/textures/Acheto.png")});
	m_Textures.insert({"Box", Achengine::Texture2D::Create("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/textures/box.png")});
	m_Textures.insert({"BoxSpecular", Achengine::Texture2D::Create("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/textures/box_specular.png")});
	std::vector<glm::vec3> locations = {{0.0f, 20.0f, 0.0f}, {0.0f, 100.0f, 0.0f}, {40.0f, 60.0f, 0.0f}, {-40.0f, 60.0f, 0.0f}, {0.0f, 60.0f, 40.0f}, {0.0f, 60.0f, -40.0f}};
	for (int i = 0; i < 6; i++)
	{
		Achengine::AActor* Actor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
		Actor->SetActorName(Achengine::format("Box_%d", i));
		Achengine::UStaticMesh* StaticMesh = new Achengine::UStaticMesh();
		StaticMesh->SetTexture(m_Textures.at("Box"));
		StaticMesh->SetSpecular(m_Textures.at("BoxSpecular"));
		Actor->SetMesh(StaticMesh);

		Actor->SetActorLocation(locations[i]);
		Actor->SetActorScale({ 10.0f, 10.0f, 10.0f});
	}

	Achengine::AActor* Light = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	Light->SetActorName("MainLight");
	Achengine::ULightMesh* LightMesh = new Achengine::ULightMesh();
	Light->SetMesh(LightMesh);
	Light->SetActorLocation(ToTransform({0.0f, 0.0f, 60.0f}));
	Light->SetActorScale({5.0f, 5.0f, 5.0f});

	//Achengine::AActor* WaterActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	//Achengine::UWaterMesh* WaterMesh = new Achengine::UWaterMesh();
	//WaterActor->SetMesh(WaterMesh);
	//WaterActor->SetActorScale({ 200.0f, 20.0f, 200.0f});

#ifdef ACHENGINE_PLATFORM_LINUX
	const std::string modelPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/models/sample.fbx";
#else
	const std::string modelPath = "assets/models/sample.fbx";
#endif

	std::strncpy(m_ModelPathBuffer, modelPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
	m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';

	if (FileExists(modelPath))
	{
		m_ModelActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
		m_ModelActor->SetActorName("ImportedModel");
		m_ModelMesh = new Achengine::UModelMesh(modelPath);
		m_ModelActor->SetMesh(m_ModelMesh);
		m_ModelActor->SetActorLocation({0.0f, 30.0f, 0.0f});
		m_ModelActor->SetActorScale(m_ModelScale);
	}
	else
	{
		ACHENGINE_WARN("No model found at {0}. Add an FBX to Sandbox/assets/models", modelPath);
	}
}

void Sandbox3D::OnDetach()
{
	for (std::pair<std::string, Achengine::Texture*> texture : m_Textures)
	{
		delete texture.second;
	}

	m_Textures.clear();
	m_ActiveActor = nullptr;
	m_SelectedActors.clear();
}

void Sandbox3D::OnUpdate(Achengine::Timestep timestep)
{
	// Update
	m_CameraController->OnUpdate(timestep);

	// Render
	Achengine::RenderCommand::SetClearColor({ 0.4f, 0.4f, 0.8f, 0.3f });
	Achengine::RenderCommand::Clear();

	Achengine::Renderer::BeginScene(m_CameraController->GetCamera());
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
	const float minPanelHeight = 140.0f;
	const float maxPanelHeight = viewport->WorkSize.y - 80.0f;
	if (m_AssetBrowserHeight < minPanelHeight)
	{
		m_AssetBrowserHeight = minPanelHeight;
	}
	if (m_AssetBrowserHeight > maxPanelHeight)
	{
		m_AssetBrowserHeight = maxPanelHeight;
	}

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
	static std::vector<std::string> modelLibraryFiles;
	static bool modelLibraryDirty = true;
	static const std::string payloadType = "MODEL_PATH_PAYLOAD";

	auto LoadModelAtPath = [&](const std::string& modelPath) {
		if (!FileExists(modelPath))
		{
			modelStatus = "Model file not found";
			return;
		}

		if (!m_ModelActor)
		{
			m_ModelActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
			m_ModelActor->SetActorName("ImportedModel");
			m_ModelMesh = new Achengine::UModelMesh(modelPath);
			m_ModelActor->SetMesh(m_ModelMesh);
			m_ModelActor->SetActorLocation({0.0f, 30.0f, 0.0f});
			m_ModelActor->SetActorScale(m_ModelScale);
			modelStatus = "Model actor created";
			return;
		}

		if (m_ModelMesh && m_ModelMesh->ReloadModel(modelPath))
		{
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

	const std::string modelRootDirectory = GetDirectoryFromPath(m_ModelPathBuffer);
	if (modelLibraryDirty)
	{
		modelLibraryFiles.clear();
		CollectModelFilesRecursive(modelRootDirectory, modelLibraryFiles);
		std::sort(modelLibraryFiles.begin(), modelLibraryFiles.end());
		modelLibraryDirty = false;
	}

	ImGui::InputText("Model Path", m_ModelPathBuffer, sizeof(m_ModelPathBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		browserDirectory = GetDirectoryFromPath(m_ModelPathBuffer);
		ImGui::OpenPopup("Model Browser");
	}
	ImGui::SameLine();
	ImGui::Button("Drop Model Here", ImVec2(140.0f, 0.0f));
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType.c_str()))
		{
			const char* droppedPath = (const char*)payload->Data;
			if (droppedPath)
			{
				std::strncpy(m_ModelPathBuffer, droppedPath, sizeof(m_ModelPathBuffer) - 1);
				m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
				LoadModelAtPath(m_ModelPathBuffer);
				modelLibraryDirty = true;
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopupModal("Model Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Current: %s", browserDirectory.c_str());
		ImGui::Separator();

		if (ImGui::Button("Up"))
		{
			const size_t slash = browserDirectory.find_last_of("/\\");
			if (slash != std::string::npos && slash > 0)
			{
				browserDirectory = browserDirectory.substr(0, slash);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::BeginChild("ModelBrowserEntries", ImVec2(620.0f, 320.0f), true);
		std::vector<FBrowserEntry> entries = ReadDirectoryEntries(browserDirectory);
		for (const FBrowserEntry& entry : entries)
		{
			std::string label = entry.IsDirectory ? "[DIR] " + entry.Name : entry.Name;
			ImGui::PushID(entry.FullPath.c_str());
			if (ImGui::Selectable(label.c_str()))
			{
				if (entry.IsDirectory)
				{
					browserDirectory = entry.FullPath;
				}
				else
				{
					std::strncpy(m_ModelPathBuffer, entry.FullPath.c_str(), sizeof(m_ModelPathBuffer) - 1);
					m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
					LoadModelAtPath(entry.FullPath);
					modelLibraryDirty = true;
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entry.IsDirectory && ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload(payloadType.c_str(), entry.FullPath.c_str(), entry.FullPath.size() + 1);
				ImGui::Text("%s", entry.Name.c_str());
				ImGui::EndDragDropSource();
			}
			ImGui::PopID();
		}
		ImGui::EndChild();

		ImGui::EndPopup();
	}

	if (ImGui::Button("Load / Reload Model"))
	{
		LoadModelAtPath(m_ModelPathBuffer);
		modelLibraryDirty = true;
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(modelStatus.c_str());

	ImGui::Separator();
	ImGui::TextUnformatted("Model Library (drag to Drop Model Here)");
	ImGui::SameLine();
	if (ImGui::Button("Refresh Library"))
	{
		modelLibraryDirty = true;
	}
	ImGui::BeginChild("ModelLibrary", ImVec2(0.0f, 150.0f), true);
	if (modelLibraryFiles.empty())
	{
		ImGui::TextUnformatted("No models found in current model directory.");
	}
	for (const std::string& modelFile : modelLibraryFiles)
	{
		const std::string displayName = ToDisplayRelativePath(modelFile, modelRootDirectory);
		ImGui::PushID(modelFile.c_str());
		if (ImGui::Selectable(displayName.c_str()))
		{
			std::strncpy(m_ModelPathBuffer, modelFile.c_str(), sizeof(m_ModelPathBuffer) - 1);
			m_ModelPathBuffer[sizeof(m_ModelPathBuffer) - 1] = '\0';
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			LoadModelAtPath(modelFile);
		}
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(payloadType.c_str(), modelFile.c_str(), modelFile.size() + 1);
			ImGui::Text("%s", displayName.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	if (ImGui::DragFloat3("Model Scale", &m_ModelScale.x, 0.05f, 0.1f, 100.0f) && m_ModelActor)
	{
		m_ModelActor->SetActorScale(m_ModelScale);
	}

	ImGui::End();

	const ImVec2 renderMin(viewport->WorkPos.x, viewport->WorkPos.y);
	const ImVec2 renderMax(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);
	const ImVec2 sceneMin(viewport->WorkPos.x, viewport->WorkPos.y);
	const ImVec2 sceneMax(viewport->WorkPos.x + assetWidth, viewport->WorkPos.y + viewport->WorkSize.y - m_AssetBrowserHeight);

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
		Achengine::EditorCamera* camera = (Achengine::EditorCamera*)m_CameraController->GetCamera();
		glm::mat4 viewProjection = camera->GetViewProjection() * camera->GetViewMatrix();
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

	Achengine::EditorCamera* camera = (Achengine::EditorCamera*)m_CameraController->GetCamera();
	const glm::mat4 viewProjection = camera->GetViewProjection() * camera->GetViewMatrix();
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
		gizmoWorldSize = glm::max(1.0f, glm::distance(camera->GetPosition(), actorPos) * 0.2f);

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
		glm::vec3 rayOrigin = camera->GetPosition();
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
				const float worldPerPixel = glm::max(0.0025f, glm::distance(camera->GetPosition(), actorPos) * 0.0015f);

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
	m_CameraController->OnEvent(event);
}
