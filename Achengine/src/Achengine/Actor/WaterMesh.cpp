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
        constexpr float upperBound = 1.0f;
        constexpr float lowerBound = upperBound * -1.0f;
        constexpr float vertexStep = 0.1f;
        constexpr float k = lowerBound + vertexStep;

        std::vector<glm::vec3> vertices;
        constexpr int indexCount = (upperBound - lowerBound) / vertexStep;
        std::vector<uint32_t> indices(indexCount*indexCount * 36);
        uint32_t ii = 0;
        for (float i = 0.0f; i <= (upperBound - lowerBound); i += vertexStep)
        {
            for (float j = 0.0f; j <= (upperBound - lowerBound); j += vertexStep)
            {
                vertices.push_back(glm::vec3(lowerBound, lowerBound + i, lowerBound + j));
                vertices.push_back(glm::vec3(lowerBound, lowerBound + i, k          + j));
                vertices.push_back(glm::vec3(lowerBound, k + i,          k          + j));
                vertices.push_back(glm::vec3(lowerBound, k + i,          k          + j));
                vertices.push_back(glm::vec3(lowerBound, k + i,          lowerBound + j));
                vertices.push_back(glm::vec3(lowerBound, lowerBound + i, lowerBound + j));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(glm::vec3(upperBound, lowerBound + i, lowerBound + j));
                vertices.push_back(glm::vec3(upperBound, lowerBound + i, k          + j));
                vertices.push_back(glm::vec3(upperBound, k + i,          k          + j));
                vertices.push_back(glm::vec3(upperBound, k + i,          k          + j));
                vertices.push_back(glm::vec3(upperBound, k + i,          lowerBound + j));
                vertices.push_back(glm::vec3(upperBound, lowerBound + i, lowerBound + j));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(glm::vec3(k + i,          lowerBound, k          + j));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound, k          + j));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound, lowerBound + j));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound, lowerBound + j));
                vertices.push_back(glm::vec3(k + i,          lowerBound, lowerBound + j));
                vertices.push_back(glm::vec3(k + i,          lowerBound, k          + j));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(glm::vec3(k + i,          upperBound, k          + j));
                vertices.push_back(glm::vec3(lowerBound + i, upperBound, k          + j));
                vertices.push_back(glm::vec3(lowerBound + i, upperBound, lowerBound + j));
                vertices.push_back(glm::vec3(lowerBound + i, upperBound, lowerBound + j));
                vertices.push_back(glm::vec3(k + i,          upperBound, lowerBound + j));
                vertices.push_back(glm::vec3(k + i,          upperBound, k          + j));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(glm::vec3(k + i,          k + j,          lowerBound));
                vertices.push_back(glm::vec3(lowerBound + i, k + j,          lowerBound));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound + j, lowerBound));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound + j, lowerBound));
                vertices.push_back(glm::vec3(k + i,          lowerBound + j, lowerBound));
                vertices.push_back(glm::vec3(k + i,          k + j,          lowerBound));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(glm::vec3(k + i,          k + j,          upperBound));
                vertices.push_back(glm::vec3(lowerBound + i, k + j,          upperBound));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound + j, upperBound));
                vertices.push_back(glm::vec3(lowerBound + i, lowerBound + j, upperBound));
                vertices.push_back(glm::vec3(k + i,          lowerBound + j, upperBound));
                vertices.push_back(glm::vec3(k + i,          k + j,          upperBound));
                for (int l = 0; l < 36; ++l)
                {
                    indices[ii] = ii; ++ii;
                }
            }
        }

        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};

        std::vector<glm::vec3> normals(indices.size());
        const std::vector<std::pair<float, float>> texCoords;
        Renderer::GenerateVertexArray(GetShaderName(), vertices, normals, texCoords, indices, Layout);
    }
}