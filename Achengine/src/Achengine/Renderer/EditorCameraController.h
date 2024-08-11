#pragma once

#include "CameraController.h"
#include "EditorCamera.h"

namespace Achengine
{
	class EditorCameraController : public CameraController
	{
	public:
		EditorCameraController(float aspectRatio);
		~EditorCameraController();

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;

	protected:
		virtual bool OnMouseScrolled(MouseScrolledEvent& e) override;
		virtual bool OnWindowResized(WindowResizeEvent& e) override;
	private:
		glm::mat4 m_ViewMatrix;
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_MousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;
	};
}