#pragma once

#include "Achengine/Renderer/Camera.h"
#include "Achengine/Core/Timestep.h"

#include "Achengine/Events/ApplicationEvent.h"
#include "Achengine/Events/MouseEvent.h"

namespace Achengine
{
	class CameraController
	{
	public:
		CameraController() = default;
		CameraController(float aspectRatio);
		~CameraController();

		virtual void OnUpdate(Timestep ts);
		virtual void OnEvent(Event& e);

		Camera* GetCamera() { return m_Camera; }
		const Camera* GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
	protected:
		virtual bool OnMouseScrolled(MouseScrolledEvent& e);
		virtual bool OnWindowResized(WindowResizeEvent& e);
	protected:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		Camera* m_Camera;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraTranslationSpeed = 5.0f;
		float m_CameraRotationSpeed = 1.0f;
	};
}