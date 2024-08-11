#pragma once
#include "Achengine.h"

#include "Camera.h"

namespace Achengine
{
	class EditorCamera : public Camera
	{
		friend class EditorCameraController;
		friend class Renderer;

	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
	protected:
		void UpdateProjection();
		void UpdateView();

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		void SetPosition(const glm::vec3& position) { m_Position = position; }
		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float ZoomSpeed() const;

		float pixel_samples_scale;  // Color scale factor for a sum of pixel samples
		glm::vec3 pixel00_loc;    // Location of pixel 0, 0
		glm::vec3   pixel_delta_u;  // Offset to pixel to the right
		glm::vec3   pixel_delta_v;  // Offset to pixel below
		glm::vec3   defocus_disk_u;       // Defocus disk horizontal radius
		glm::vec3   defocus_disk_v;       // Defocus disk vertical radius
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}