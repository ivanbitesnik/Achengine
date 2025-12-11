#include "Achenginepch.h"
#include "WaterMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"

namespace Achengine
{
    UWaterMesh::UWaterMesh()
    {
#ifdef ACHENGINE_PLATFORM_LINUX
        m_ShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/BasicWater.glsl";
#else
        m_ShaderPath = "assets/shaders/BasicWater.glsl";
#endif
        Initialize();
    }

    void UWaterMesh::SetUniforms()
    {
        constexpr int numWaves = 4;
        const std::string& ShaderName = GetObjectNameFromFilePath(m_ShaderPath);
        Renderer::SetShaderUniform(ShaderName, "u_NumWaves", numWaves);
        Renderer::SetShaderUniform(ShaderName, "u_envMap", 0);
		for (int i = 0; i < numWaves; ++i) 
        {
            float amplitude = 0.5f / (i + 1);
            Renderer::SetShaderUniform(ShaderName, format("u_Amplitude[%d]", i), amplitude);

            float wavelength = 8 * M_PI / (i + 1);
            Renderer::SetShaderUniform(ShaderName, format("u_Wavelength[%d]", i), wavelength);

            float speed = 1.0f + 2*i;
            Renderer::SetShaderUniform(ShaderName, format("u_Speed[%d]", i), speed);
            
            float angle = uniformRandomInRange(-M_PI/3, M_PI/3);
            Renderer::SetShaderUniform(ShaderName, format("u_Direction[%d]", i), {cos(angle), sin(angle)});
        }
    }

    void UWaterMesh::GenerateVertexArray()
    {
        if (Renderer::GetVertexArray(GetShaderName()))
        {
            return;
        }

        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        int ii = 0;
        for (float i = 0.01f; i <= 1.0f ; i += 0.01f)
        {
            vertices.push_back(glm::vec3(-i, -i, -i));
            vertices.push_back(glm::vec3( i, -i, -i)); 
            vertices.push_back(glm::vec3( i,  i, -i));
            vertices.push_back(glm::vec3( i,  i, -i));
            vertices.push_back(glm::vec3(-i,  i, -i));
            vertices.push_back(glm::vec3(-i, -i, -i));
            /////////////////////////////////////////
            vertices.push_back(glm::vec3(-i, -i,  i));
            vertices.push_back(glm::vec3( i, -i,  i));
            vertices.push_back(glm::vec3( i,  i,  i));
            vertices.push_back(glm::vec3( i,  i,  i));
            vertices.push_back(glm::vec3(-i,  i,  i));
            vertices.push_back(glm::vec3(-i, -i,  i));
            /////////////////////////////////////////
            vertices.push_back(glm::vec3(-i,  i,  i));
            vertices.push_back(glm::vec3(-i,  i, -i));
            vertices.push_back(glm::vec3(-i, -i, -i));
            vertices.push_back(glm::vec3(-i, -i, -i));
            vertices.push_back(glm::vec3(-i, -i,  i));
            vertices.push_back(glm::vec3(-i,  i,  i));
            /////////////////////////////////////////
            vertices.push_back(glm::vec3( i,  i,  i));
            vertices.push_back(glm::vec3( i,  i, -i));
            vertices.push_back(glm::vec3( i, -i, -i)); 
            vertices.push_back(glm::vec3( i, -i, -i)); 
            vertices.push_back(glm::vec3( i, -i,  i));
            vertices.push_back(glm::vec3( i,  i,  i));
            /////////////////////////////////////////
            vertices.push_back(glm::vec3(-i, -i, -i));
            vertices.push_back(glm::vec3( i, -i, -i)); 
            vertices.push_back(glm::vec3( i, -i,  i));
            vertices.push_back(glm::vec3( i, -i,  i));
            vertices.push_back(glm::vec3(-i, -i,  i));
            vertices.push_back(glm::vec3(-i, -i, -i));
            /////////////////////////////////////////
            vertices.push_back(glm::vec3(-i,  i, -i));
            vertices.push_back(glm::vec3( i,  i, -i));
            vertices.push_back(glm::vec3( i,  i,  i));
            vertices.push_back(glm::vec3( i,  i,  i));
            vertices.push_back(glm::vec3(-i,  i,  i));
            vertices.push_back(glm::vec3(-i,  i, -i));
            for (int j = 0; j < 36; ++j)
            {
                indices.push_back(ii); ++ii;
            }
        }

        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};

        std::vector<glm::vec3> normals;
        std::vector<std::pair<float, float>> texCoords;
        Renderer::GenerateVertexArray(GetShaderName(), vertices, normals, texCoords, indices, Layout);
    }
}