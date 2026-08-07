#pragma once

#include <glm/glm.hpp>

namespace Achengine
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}

		virtual ~Camera() = default;

		virtual const glm::vec3& GetPosition() const { return m_Position; }
		virtual const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		virtual glm::mat4 GetViewProjection() const { return m_Projection; }

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}