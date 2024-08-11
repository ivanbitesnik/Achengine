#pragma once

#include "Achengine/Renderer/OrthographicCamera.h"
#include "Achengine/Core/Timestep.h"

#include "Achengine/Events/ApplicationEvent.h"
#include "Achengine/Events/MouseEvent.h"

namespace Achengine
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio);
		~OrthographicCameraController();

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera* GetCamera() { return m_Camera; }
		const OrthographicCamera* GetCamera() const { return m_Camera; }

	protected:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	protected:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera* m_Camera;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;

		float m_CameraTranslationSpeed = 5.0f;
		float m_CameraRotationSpeed = 180.0f;
	};
}