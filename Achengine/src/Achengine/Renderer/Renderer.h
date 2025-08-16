#pragma once

#include "Camera.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Texture.h"

class Actor;

namespace Achengine
{
    struct MeshMaterial
    {
        public:
            MeshMaterial(const glm::vec3& color, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess) :
                color(color), ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}
            glm::vec3 color = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 ambient = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
            float shininess = 32.0f;
    };

    struct LightSource
    {
        public:
            LightSource(const glm::vec3& position) : position(position) {}
            LightSource(const glm::vec3& position, const glm::vec3& color, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) :
                position(position), color(color), ambient(ambient), diffuse(diffuse), specular(specular) {}
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 ambient = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
    };

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

        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const MeshMaterial material, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));
        static void DrawCube(const glm::vec3& position, const glm::vec3& size, const Texture* texture, const Texture* specularMap = nullptr, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));

        static void DrawLight(const LightSource lightSource, const glm::vec3& size, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f));

		inline static std::shared_ptr<RendererAPI::API> GetAPI() { return RendererAPI::GetAPI(); }
	};

#ifdef ACHENGINE_PLATFORM_LINUX
	static std::string TextureShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/Texture.glsl";
	static std::string CubeShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/Cube.glsl";
	static std::string LightSourceShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/LightSource.glsl";
#else
	static std::string TextureShaderPath = "assets/shaders/Texture.glsl";
	static std::string CubeShaderPath = "assets/shaders/Cube.glsl";
	static std::string LightSourceShaderPath = "assets/shaders/LightSource.glsl";
#endif

    static float quadVertexArray[5 * 4] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.5f, 0.0f,
        0.5f,  0.5f, 0.0f, 0.5f, 0.5f,
        -0.5f,  0.5f, 0.0f, 0.0f, 0.5f
    };

    static unsigned int quadIndexArray[] = { 0, 1, 2, 2, 3, 0 };

    static float cubeVertexArray[6 * 3 * 6] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    static float cubeWithNormalsVertexArray[8 * 6 * 6] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
}