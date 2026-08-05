#pragma once

#include "Achengine.h"
#include "Camera.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Texture.h"

namespace Achengine
{
    class AActor;

    struct FSceneLight
    {
        glm::vec3 Position = glm::vec3(0.0f);
        glm::vec3 Ambient = glm::vec3(0.2f);
        glm::vec3 Diffuse = glm::vec3(0.5f);
        glm::vec3 Specular = glm::vec3(1.0f);
        float Constant = 1.0f;
        float Linear = 0.014f;
        float Quadratic = 0.0007f;
    };

    struct RendererStorage
	{
        glm::vec3 CameraPosition;
        glm::mat4 ViewProjectionMatrix;
		std::vector<FSceneLight> SceneLights;
		Texture2D* WhiteTexture;
        std::map<std::string, VertexArray*> VertexArrays;
        std::vector<Shader*> Shaders;

        Shader* GetShader(const std::string& ShaderName)
        {
            for (Shader* shader : Shaders)
            {
                if (shader->GetName() == ShaderName)
                {
                    return shader;
                }
            }

            return nullptr;
        }
	};
    
    class UMesh;

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

        static Shader* AddShader(const std::string& ShaderPath);
        static void SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, int value);
        static void SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, float value);
        static void SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec2& value);
        static void SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec3& value);
        static void SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec4& value);
        static void SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::mat4& value);

        template<unsigned int N>
        static VertexArray* AddVertexArray(const std::string& VertexArrayName, float (&VertexCoords)[N], const BufferLayout& BufferLayout);
        static VertexArray* AddVertexArray(const std::string& VertexArrayName, const std::vector<float>& VertexCoords, const BufferLayout& BufferLayout);
        template<unsigned int N>
        static void AddIndexBufferToArray(const std::string& VertexArrayName, uint32_t (&Indices)[N]);
        static void AddIndexBufferToArray(const std::string& VertexArrayName, const std::vector<uint32_t>& Indices);
        static VertexArray* GetVertexArray(const std::string& VertexArrayName);

        static void GenerateNormals(const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices, std::vector<Vector3>& normals);
        static void GenerateVertexArray(const std::string& VertexId, const std::vector<Vector3>& vertices, std::vector<Vector3>& normals,
             const std::vector<std::pair<float, float>>& texCoords, const std::vector<uint32_t>& indices, const BufferLayout& Layout);

		static void OnWindowResize(uint32_t width, uint32_t height);
        
		static void BeginScene(Camera* camera);
		static void EndScene();

		static constexpr uint32_t MaxSceneLights = 16;
        static void AddSceneLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular,
            float constant = 1.0f, float linear = 0.014f, float quadratic = 0.0007f);
        static void ClearSceneLights();
        
		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture2D* texture, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f), const glm::vec4& tint = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D* texture, const float angle = 0.0f, const glm::vec3& rot = glm::vec3(1.0f), const glm::vec4& tint = glm::vec4(1.0f));
        
        static void DrawVertexArray(const std::string& VertexArrayName);
        static void DrawMesh(UMesh* Mesh);

		inline static std::shared_ptr<RendererAPI::API> GetAPI() { return RendererAPI::GetAPI(); }
	};

#ifdef ACHENGINE_PLATFORM_LINUX
	static std::string TextureShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/Texture.glsl";
#else
	static std::string TextureShaderPath = "assets/shaders/Texture.glsl";
#endif


    static float quadVertexArray[5 * 4] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
    };

    static unsigned int quadIndexArray[] = { 0, 1, 2, 2, 3, 0 };

    static float cubeVertexArray[6 * 3 * 6] = {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f
    };
}