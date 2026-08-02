#include "Sandbox3D.h"
// ----------------------------------------

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

static glm::vec3 ToTransform(glm::vec3 vec)
{
	return glm::vec3(vec.y, vec.z, vec.x);
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
		Achengine::UStaticMesh* StaticMesh = new Achengine::UStaticMesh();
		StaticMesh->SetTexture(m_Textures.at("Box"));
		StaticMesh->SetSpecular(m_Textures.at("BoxSpecular"));
		Actor->SetMesh(StaticMesh);

		Actor->SetActorLocation(locations[i]);
		Actor->SetActorScale({ 10.0f, 10.0f, 10.0f});
	}

	Achengine::AActor* Light = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	Light->SetMesh(new Achengine::ULightMesh());
	Light->SetActorLocation(ToTransform({0.0f, 0.0f, 60.0f}));
	Light->SetActorScale({5.0f, 5.0f, 5.0f});

	Achengine::AActor* WaterActor = Achengine::WorldActorCache::SpawnActor<Achengine::AActor>();
	Achengine::UWaterMesh* WaterMesh = new Achengine::UWaterMesh();
	WaterActor->SetMesh(WaterMesh);
	WaterActor->SetActorScale({ 200.0f, 20.0f, 200.0f});
}

void Sandbox3D::OnDetach()
{
	for (std::pair<std::string, Achengine::Texture*> texture : m_Textures)
	{
		delete texture.second;
	}

	m_Textures.clear();
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
}

void Sandbox3D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", m_SquareColor);
	ImGui::End();
}

void Sandbox3D::OnEvent(Achengine::Event& event)
{
	m_CameraController->OnEvent(event);
}
