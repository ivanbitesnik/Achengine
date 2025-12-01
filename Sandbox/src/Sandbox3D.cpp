#include "Sandbox3D.h"
// ----------------------------------------

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	m_Textures.insert(std::pair<std::string, Achengine::Texture2D*>("Acheto", Achengine::Texture2D::Create("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/textures/Acheto.png")));
	m_Textures.insert(std::pair<std::string, Achengine::Texture2D*>("Box", Achengine::Texture2D::Create("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/textures/box.png")));
	m_Textures.insert(std::pair<std::string, Achengine::Texture2D*>("BoxSpecular", Achengine::Texture2D::Create("/home/acheto/Desktop/projects/Achengine/Sandbox/assets/textures/box_specular.png")));
	
	std::vector<glm::vec3> locations = {{0.0f, 10.0f, 0.0f}, {0.0f, 50.0f, 0.0f}, {20.0f, 30.0f, 0.0f}, {-20.0f, 30.0f, 0.0f}, {0.0f, 30.0f, 20.0f}, {0.0f, 30.0f, -20.0f}};
	for (int i = 0; i < 6; i++)
	{
		Achengine::AActor* Actor = new Achengine::AActor();
		Achengine::UStaticMesh* StaticMesh = new Achengine::UStaticMesh();
		StaticMesh->SetTexture(m_Textures.at("Box"));
		StaticMesh->SetSpecular(m_Textures.at("BoxSpecular"));
		Actor->SetStaticMesh(StaticMesh);

		Actor->SetActorLocation(locations[i]);
		Actor->SetActorScale({ 10.0f, 10.0f, 10.0f});
	}

	Achengine::AActor* Light = new Achengine::AActor();
	Light->SetStaticMesh(new Achengine::ULightMesh());
	Light->SetActorLocation(ToTransform({0.0f, 0.0f, 30.0f}));
	Light->SetActorScale({5.0f, 5.0f, 5.0f});

	Achengine::AActor* WaterActor = new Achengine::AActor();
	Achengine::UWaterMesh* WaterMesh = new Achengine::UWaterMesh();
	WaterMesh->SetTexture(m_Textures.at("Acheto"));
	WaterMesh->AddSineFunction(new Achengine::FSineFunction());
	WaterActor->SetStaticMesh(WaterMesh);
	WaterActor->SetActorLocation(ToTransform({ 0.0f, 0.0f, 0.0f }));
	WaterActor->SetActorRotation({1.0f, 0.0f, 0.0f}, 90.0f);
	WaterActor->SetActorScale({ 100.0f, 100.0f, 100.0f});
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
	Achengine::RenderCommand::SetClearColor({ 0.0f, 0.4f, 0.8f, 0.3f });
	Achengine::RenderCommand::Clear();

	pos = glm::mod(pos, 360.0f);
	pos += 1.0f * timestep;
	const float x = cos(pos) * 30.0f;
	const float y = sin(pos) * 30.0f;
	angle = glm::mod(angle, 360.0f);
	angle += 30.0f * timestep;

	lightColorRate = glm::mod(lightColorRate, 360.0f);
	lightColorRate += 0.5f * timestep;
	const glm::vec3 lightColor = {sin(lightColorRate * 2.0f), sin(lightColorRate * 0.7f), sin(lightColorRate * 1.3f)};
	const glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
	const glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
	const glm::vec3 specular({0.5f, 0.5f, 0.5f});

	Achengine::Renderer::BeginScene(m_CameraController->GetCamera());
	if (Achengine::WorldActorCache* Cache = Achengine::WorldActorCache::Get())
	{
		for (Achengine::AActor* Actor : Cache->GetActorCache())
		{
			//Actor->SetActorRotation({0.0f, 1.0f, 0.0f}, angle);
			Achengine::Renderer::DrawActor(Actor);
		}
	}

	//const Achengine::LightSource lightSource = Achengine::LightSource(ToTransform({0.0f, 0.0f, 60.0f}));
	//
	//Achengine::Renderer::BeginScene(m_CameraController->GetCamera());
	//Achengine::Renderer::DrawLight(lightSource, { 5.0f, 5.0f, 5.0f });
	//Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, 0.0f, -5.0f }), { 50.0f, 50.0f }, { 0.1f, 0.1f, 0.1f, 1.0f }, 90.0f, { 1.0f, 0.0f, 0.0f });
	//Achengine::Renderer::DrawCube(ToTransform({ 0.0f, 0.0f, 10.0f }), { 20.0f, 20.0f, 20.0f }, m_Textures.at("Box"), m_Textures.at("BoxSpecular"), angle, ToTransform({0.0f, 0.0f, 1.0f}));
	//Achengine::Renderer::DrawCube(ToTransform({ 0.0f, 0.0f, 100.0f }), { 20.0f, 20.0f, 20.0f }, m_Textures.at("Box"), m_Textures.at("BoxSpecular"), angle, ToTransform({0.0f, 0.0f, 1.0f}));
	//Achengine::Renderer::DrawCube(ToTransform({ 50.0f, 0.0f, 60.0f }), { 20.0f, 20.0f, 20.0f }, m_Textures.at("Box"), m_Textures.at("BoxSpecular"), -angle, ToTransform({1.0f, 0.0f, 0.0f}));
	//Achengine::Renderer::DrawCube(ToTransform({ 0.0f, 50.0f, 60.0f }), { 20.0f, 20.0f, 20.0f }, m_Textures.at("Box"), m_Textures.at("BoxSpecular"), angle, ToTransform({0.0f, 1.0f, 0.0f}));
	//Achengine::Renderer::DrawCube(ToTransform({ -50.0f, 0.0f, 60.0f }), { 20.0f, 20.0f, 20.0f }, m_Textures.at("Box"), m_Textures.at("BoxSpecular"), -angle, ToTransform({1.0f, 0.0f, 0.0f}));
	//Achengine::Renderer::DrawCube(ToTransform({ 0.0f, -50.0f, 60.0f }), { 20.0f, 20.0f, 20.0f }, m_Textures.at("Box"), m_Textures.at("BoxSpecular"), angle, ToTransform({0.0f, 1.0f, 0.0f}));
	Achengine::Renderer::EndScene();
}

void Sandbox3D::OnImGuiRender()
{
	//ImGui::Begin("Settings");
	//ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	//ImGui::End();
}

void Sandbox3D::OnEvent(Achengine::Event& event)
{
	m_CameraController->OnEvent(event);
}
