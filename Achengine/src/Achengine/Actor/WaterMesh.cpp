#include "Achenginepch.h"
#include "WaterMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/MeshDrawable.h"

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

    void UWaterMesh::GenerateMeshDrawable()
    {
        std::vector<glm::vec3> vertices(9000);
        std::vector<uint32_t> indices(9000*6);

        const glm::vec3& location = {0.0f, 0.0f, 0.0f};
        const glm::vec3& scale = {1.0f, 1.0f, 1.0f};

        int vi = 0, ii = 0;
        for (int x = location.x - 10*scale.x; x < location.x + 10*scale.x; ++x)
        {
            for (int y = location.y - 10*scale.y; y < location.y + 10*scale.y; ++y)
            {
                const float height = location.z + 10*scale.z;
                vertices[vi++] = glm::vec3(x, y, height);
                vertices[vi++] = glm::vec3(x, y + 1, height);
                vertices[vi++] = glm::vec3(x + 1, y, height);
                vertices[vi++] = glm::vec3(x + 1, y, height);
                vertices[vi++] = glm::vec3(x, y + 1, height);
                vertices[vi++] = glm::vec3(x + 1, y + 1, height);
                for (int j = 0; j < 6; ++j)
                {
                    indices[ii] = ii;
                    ++ii;
                }
            }
        }

        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};

        std::vector<glm::vec3> normals;
        std::vector<std::pair<float, float>> texCoords;
        new UMeshDrawable(GetShaderName(), vertices, normals, texCoords, indices, Layout);
    }
}