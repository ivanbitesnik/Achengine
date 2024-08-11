#include "Achenginepch.h"
#include "CameraController.h"

#include "Achengine/Core/Input.h"
#include "Achengine/Core/KeyCodes.h"

namespace Achengine
{
	CameraController::CameraController(float aspectRatio)
		: m_AspectRatio(aspectRatio)
	{
		m_Camera = new Camera;
	}

	CameraController::~CameraController()
	{
		delete m_Camera;
	}

	void CameraController::OnUpdate(Timestep ts)
	{
	}

	void CameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(CameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(CameraController::OnWindowResized));
	}

	bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		return false;
	}

	bool CameraController::OnWindowResized(WindowResizeEvent& e)
	{
		return false;
	}
}