#include "Achenginepch.h"
#include "OrthographicCameraController.h"

#include "Achengine/Core/Input.h"
#include "Achengine/Core/KeyCodes.h"

#include "OrthographicCamera.h"

namespace Achengine
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio)
		: m_AspectRatio(aspectRatio)
	{
		m_Camera = new OrthographicCamera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	OrthographicCameraController::~OrthographicCameraController()
	{
		delete m_Camera;
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		m_CameraTranslationSpeed = m_ZoomLevel * 5.0f;

		if (Input::IsKeyPressed(ACHENGINE_KEY_A))
		{
			m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_D))
		{
			m_CameraPosition.x += m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_S))
		{
			m_CameraPosition.y -= m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_W))
		{
			m_CameraPosition.y += m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_LEFT))
		{
			m_CameraRotation += m_CameraRotationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_RIGHT))
		{
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}

		m_Camera->SetPosition(m_CameraPosition);
		m_Camera->SetRotation(m_CameraRotation);
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera->SetProjectionMatrix(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera->SetProjectionMatrix(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}
}