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
	m_Texture = Achengine::Texture2D::Create("/home/acheto/Desktop/engine/Achengine/Sandbox/assets/textures/Acheto.png");
}

void Sandbox3D::OnDetach()
{
	delete m_Texture;
}

void Sandbox3D::OnUpdate(Achengine::Timestep timestep)
{
	// Update
	m_CameraController->OnUpdate(timestep);

	// Render
	Achengine::RenderCommand::SetClearColor({ 0.0f, 0.4f, 1.0f, 0.5f });
	Achengine::RenderCommand::Clear();

	pos = glm::mod(pos, 360.0f);
	//pos += 2.0f * timestep;
	const float x = cos(pos) * 15.0f;
	const float y = sin(pos) * 15.0f;
	
	lightColorRate += 0.5f * timestep;
	glm::vec3 lightColor = {sin(lightColorRate * 0.7f), sin(lightColorRate * 1.3f), sin(lightColorRate * 1.7f)};
	glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.4f);
	Achengine::LightSource lightSource = Achengine::LightSource(ToTransform({ x, y, -4.0f}), lightColor, ambientColor, diffuseColor, {1.0f, 1.0f, 1.0f});
	
	Achengine::Renderer::BeginScene(m_CameraController->GetCamera());
	Achengine::Renderer::DrawLight(lightSource, { 2.0f, 2.0f, 2.0f });
	Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, 0.0f, -5.0f }), { 50.0f, 50.0f }, { 0.1f, 0.1f, 0.1f, 1.0f }, 90.0f, { 1.0f, 0.0f, 0.0f });
	Achengine::MeshMaterial emeraldMaterial = Achengine::MeshMaterial({ 1.0f, 1.0f, 1.0f }, {0.0215f, 0.1745f, 0.0215f}, {0.07568f, 0.61424f, 0.07568f}, {0.633f, 0.727811f, 0.633f}, 2.0f);
	Achengine::Renderer::DrawCube(ToTransform({ 0.0f, 0.0f, -2.5f }), { 5.0f, 5.0f, 5.0f }, emeraldMaterial);
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
