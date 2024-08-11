#include "Sandbox2D.h"
// --- Entry Point ------------------------
//#include "Achengine/Core/EntryPoint.h"
// ----------------------------------------

#include "ImGui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D")
{
	m_CameraController = new Achengine::OrthographicCameraController(1280.0f / 720.0f);
}

Sandbox2D::~Sandbox2D()
{
	delete m_CameraController;
}

void Sandbox2D::OnAttach()
{
	m_Texture = Achengine::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	delete m_Texture;
}

void Sandbox2D::OnUpdate(Achengine::Timestep timestep)
{
	// Update
	m_CameraController->OnUpdate(timestep);

	// Render
	Achengine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Achengine::RenderCommand::Clear();

	angle = glm::mod(angle, 360.0f);
	angle += 0.5f;
	pos = glm::mod(pos, 360.0f);
	pos += 0.01f;
	const float x = glm::cos(pos);
	const float y = glm::sin(pos);
	Achengine::Renderer2D::BeginScene(m_CameraController->GetCamera());
	Achengine::Renderer2D::DrawQuad({ 1.0f/x, 1.0f/x }, { 1.0f, 1.0f }, m_SquareColor, angle, {1.0f, 1.0f, 1.0f});
	Achengine::Renderer2D::DrawQuad({ x, x }, { 1.0f, 1.0f }, glm::vec4(1.0f) - glm::vec4(m_SquareColor.r, m_SquareColor.g, m_SquareColor.b, 0.0f), angle, {1.0f, 1.0f, 1.0f});
	Achengine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.5f }, { 10.0f, 10.0f }, m_Texture);
	Achengine::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Achengine::Event& event)
{
	m_CameraController->OnEvent(event);
}
