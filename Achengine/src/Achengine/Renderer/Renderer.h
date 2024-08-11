#pragma once

#include "Camera.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Texture.h"

class Actor;

namespace Achengine
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(Camera* camera);
		static void EndScene();

		static void Submit(const VertexArray* vertexArray, Shader* shader, const glm::mat4 transform = glm::mat4(1.0f));
		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture2D* texture, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f), const glm::vec4& tint = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D* texture, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f), const glm::vec4& tint = glm::vec4(1.0f));

        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));

        static void DrawLight(const glm::vec3& position, const glm::vec3& size, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));

		inline static std::shared_ptr<RendererAPI::API> GetAPI() { return RendererAPI::GetAPI(); }
	};

    static float* CreateQuadVertexArray(int& arraySize)
    {
        float vertexArray[] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
        };

        arraySize = sizeof(vertexArray);

        return vertexArray;
    }

    static unsigned int* CreateQuadIndexArray(int& arraySize)
    {
        unsigned int indexArray[] = { 0, 1, 2, 2, 3, 0 };

        arraySize = sizeof(indexArray);

        return indexArray;
    }

    static float* CreateCubeVertexArray(int& arraySize)
    {
        float vertexArray[] = {
            -1.0f,  1.0f,  1.0f,   // Top left (back face)
             1.0f,  1.0f,  1.0f,   // Top right (back face)
            -1.0f, -1.0f,  1.0f,   // Bottom left (back face)
             1.0f, -1.0f,  1.0f,   // Bottom right (back face)
            -1.0f,  1.0f, -1.0f,   // Top left (front face)
             1.0f,  1.0f, -1.0f,   // Top right (front face)
            -1.0f, -1.0f, -1.0f,   // Bottom left (front face)
             1.0f, -1.0f, -1.0f    // Bottom right (front face)
        };

        arraySize = sizeof(vertexArray);

        return vertexArray;
    }

    static float* CreateNormalsVertexArray(int& arraySize)
    {
        float vertexArray[] = {
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f
        };

        arraySize = sizeof(vertexArray);

        return vertexArray;
    }

    static unsigned int* CreateCubeIndexArray(int& arraySize)
    {
        unsigned int indexArray[] = {
            0, 2, 3, 0, 1, 3,   // Back face
            3, 7, 6, 3, 2, 6,   // Bottom face
            6, 7, 5, 6, 4, 5,   // Front face
            5, 7, 3, 5, 1, 3,   // Right face
            0, 2, 6, 0, 4, 6,   // Left face
            0, 4, 5, 0, 1, 5    // Top face
        };

        arraySize = sizeof(indexArray);

        return indexArray;
    }
}