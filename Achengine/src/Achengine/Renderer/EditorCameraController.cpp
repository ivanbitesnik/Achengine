#include "Achenginepch.h"
#include "EditorCameraController.h"

#include "Achengine/Core/Input.h"
#include "Achengine/Core/KeyCodes.h"
#include "Achengine/Core/MouseButtonCodes.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Achengine
{
	EditorCameraController::EditorCameraController(float aspectRatio)
	{
		m_Camera = new EditorCamera(65.0f, aspectRatio, 0.001f, 1000.0f);
	}

	EditorCameraController::~EditorCameraController()
	{
	}

	void EditorCameraController::OnUpdate(Timestep ts)
	{
		EditorCamera* camera = (EditorCamera*)m_Camera;

		const float CameraYaw = glm::radians(camera->GetYaw());
		const float CameraPitch = glm::radians(camera->GetPitch());
		if (Input::IsKeyPressed(ACHENGINE_KEY_A))
		{
			const glm::vec3 CameraRightDirection = camera->GetRightDirection();
			camera->m_FocalPoint.x -= CameraRightDirection.x * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.y -= CameraRightDirection.y * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.z -= CameraRightDirection.z * m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_D))
		{
			const glm::vec3 CameraRightDirection = camera->GetRightDirection();
			camera->m_FocalPoint.x += CameraRightDirection.x * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.y += CameraRightDirection.y * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.z += CameraRightDirection.z * m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_S))
		{
			const glm::vec3 CameraForwardDirection = camera->GetForwardDirection();
			camera->m_FocalPoint.x -= CameraForwardDirection.x * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.y -= CameraForwardDirection.y * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.z -= CameraForwardDirection.z * m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(ACHENGINE_KEY_W))
		{
			const glm::vec3 CameraForwardDirection = camera->GetForwardDirection();
			camera->m_FocalPoint.x += CameraForwardDirection.x * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.y += CameraForwardDirection.y * m_CameraTranslationSpeed * ts;
			camera->m_FocalPoint.z += CameraForwardDirection.z * m_CameraTranslationSpeed * ts;
		}

		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = (mouse - m_MousePosition) * m_CameraRotationSpeed;
		m_MousePosition = mouse;

		if (Input::IsMouseButtonPressed(ACHENGINE_MOUSE_BUTTON_MIDDLE))
		{
			camera->MousePan(delta);
		}
		else if (Input::IsMouseButtonPressed(ACHENGINE_MOUSE_BUTTON_RIGHT))
		{
			camera->MouseRotate(delta);
		}

		camera->UpdateView();
	}

	void EditorCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCameraController::OnMouseScrolled));
	}

	bool EditorCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		EditorCamera* camera = (EditorCamera*)m_Camera;
		camera->UpdateProjection();

		return false;
	}

	bool EditorCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (Input::IsMouseButtonPressed(ACHENGINE_MOUSE_BUTTON_RIGHT))
		{
			m_CameraTranslationSpeed += e.GetYOffset() * 2.0f;
			m_CameraTranslationSpeed = glm::clamp(m_CameraTranslationSpeed, 0.1f, 1000.0f);
		}
		else
		{
			EditorCamera* camera = (EditorCamera*)m_Camera;

			float delta = e.GetYOffset() * 0.1f;
			camera->MouseZoom(delta);
			camera->UpdateView();
		}

		return false;
	}
}