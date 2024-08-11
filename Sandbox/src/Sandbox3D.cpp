#include "Sandbox3D.h"
// --- Entry Point ------------------------
#include "Achengine/Core/EntryPoint.h"
// ----------------------------------------

#include "ImGui/imgui.h"
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
	m_Texture = Achengine::Texture2D::Create("assets/textures/Acheto.png");
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

	angle = glm::mod(angle, 360.0f);
	angle += 0.5f;
	pos = glm::mod(pos, 360.0f);
	pos += 0.01f;
	const float x = glm::cos(pos);
	const float y = glm::sin(pos);

	Achengine::Renderer::BeginScene(m_CameraController->GetCamera());
	Achengine::Renderer::DrawLight(ToTransform({ -30.0f, 0.0f, 0.0f }), { 5.0f, 5.0f, 5.0f });
	Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, 0.0f, -5.0f }), { 50.0f, 50.0f }, { 0.1f, 0.1f, 0.1f, 1.0f }, 90.0f, { 1.0f, 0.0f, 0.0f });
	Achengine::Renderer::DrawCube(ToTransform({ -30.0f, 20.0f, 0.0f }), { 5.0f, 5.0f, 5.0f }, { 1.0f, 0.5f, 0.31f });

	//Achengine::Renderer::DrawQuad({ 1.0f / x, 1.0f / x, 0.0f }, { 3.0f, 3.0f }, m_SquareColor, angle, { 1.0f, 1.0f, 1.0f });
	//Achengine::Renderer::DrawQuad({ x , x, 0.0f }, { 3.0f, 3.0f }, glm::vec4(1.0f) - glm::vec4(m_SquareColor.r, m_SquareColor.g, m_SquareColor.b, 0.0f), angle, { 1.0f, 1.0f, 1.0f });
	//Achengine::Renderer::DrawQuad(ToTransform({ 10.0f, 0.0f, 0.0f }), { 10.0f, 10.0f}, m_Texture);
	//Achengine::Renderer::DrawQuad(ToTransform({ -10.0f, 0.0f, 0.0f }), { 10.0f, 10.0f }, m_Texture);
	//Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, 10.0f, 0.0f }), { 10.0f, 10.0f }, m_Texture, 90.0f, { 0.0f, 1.0f, 0.0f });
	//Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, -10.0f, 0.0f }), { 10.0f, 10.0f }, m_Texture, 90.0f, { 0.0f, 1.0f, 0.0f });
	//Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, 0.0f, 10.0f }), { 10.0f, 10.0f }, m_Texture, 90.0f, { 1.0f, 0.0f, 0.0f });
	//Achengine::Renderer::DrawQuad(ToTransform({ 0.0f, 0.0f, -10.0f }), { 10.0f, 10.0f }, m_Texture, 90.0f, { 1.0f, 0.0f, 0.0f });
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
