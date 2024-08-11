#pragma once

#include "OrthographicCamera.h"
#include "Texture.h"

namespace Achengine
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(OrthographicCamera* camera);
		static void EndScene();

		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture2D* texture, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f), const glm::vec4& tint = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D* texture, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f), const glm::vec4& tint = glm::vec4(1.0f));
	};
}