#include "Achenginepch.h"
#include "WaterMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"

namespace Achengine
{
    namespace
    {
        const bool s_RegisteredWaterMeshClass = UActorComponent::RegisterComponentClass<UWaterMesh>("UWaterMesh", "water");
    }

    UWaterMesh::UWaterMesh()
    {
#ifdef ACHENGINE_PLATFORM_LINUX
        m_ShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/BasicWater.glsl";
#else
        m_ShaderPath = "assets/shaders/BasicWater.glsl";
#endif
    		m_Bounds.Center = glm::vec3(0.0f, 0.0f, 0.0f);
    		m_Bounds.Extents = glm::vec3(1.0f, 1.0f, 1.0f);
    		m_Bounds.SphereRadius = glm::length(m_Bounds.Extents);
    		m_Bounds.IsValid = true;
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
        constexpr int cellCount = (int)((upperBound - lowerBound) / vertexStep);

        std::vector<Vector3> vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(cellCount * cellCount * 36);
        indices.reserve(cellCount * cellCount * 36);
        uint32_t ii = 0;
        for (int i = 0; i < cellCount; ++i)
        {
            const float y0 = lowerBound + i * vertexStep;
            const float y1 = y0 + vertexStep;
            for (int j = 0; j < cellCount; ++j)
            {
                const float z0 = lowerBound + j * vertexStep;
                const float z1 = z0 + vertexStep;

                vertices.push_back(Vector3(lowerBound, y0, z0));
                vertices.push_back(Vector3(lowerBound, y0, z1));
                vertices.push_back(Vector3(lowerBound, y1, z1));
                vertices.push_back(Vector3(lowerBound, y1, z1));
                vertices.push_back(Vector3(lowerBound, y1, z0));
                vertices.push_back(Vector3(lowerBound, y0, z0));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(Vector3(upperBound, y0, z0));
                vertices.push_back(Vector3(upperBound, y0, z1));
                vertices.push_back(Vector3(upperBound, y1, z1));
                vertices.push_back(Vector3(upperBound, y1, z1));
                vertices.push_back(Vector3(upperBound, y1, z0));
                vertices.push_back(Vector3(upperBound, y0, z0));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(Vector3(y1, lowerBound, z1));
                vertices.push_back(Vector3(y0, lowerBound, z1));
                vertices.push_back(Vector3(y0, lowerBound, z0));
                vertices.push_back(Vector3(y0, lowerBound, z0));
                vertices.push_back(Vector3(y1, lowerBound, z0));
                vertices.push_back(Vector3(y1, lowerBound, z1));
                ////////////////////////////////////////////////////////////////////////
                vertices.push_back(Vector3(z1, upperBound, y1));
                vertices.push_back(Vector3(z0, upperBound, y1));
                vertices.push_back(Vector3(z0, upperBound, y0));
                vertices.push_back(Vector3(z0, upperBound, y0));
                vertices.push_back(Vector3(z1, upperBound, y0));
                vertices.push_back(Vector3(z1, upperBound, y1));
                ////////////////////////////////////////////////////////////////////////
                vertices.push_back(Vector3(y1, z1, lowerBound));
                vertices.push_back(Vector3(y0, z1, lowerBound));
                vertices.push_back(Vector3(y0, z0, lowerBound));
                vertices.push_back(Vector3(y0, z0, lowerBound));
                vertices.push_back(Vector3(y1, z0, lowerBound));
                vertices.push_back(Vector3(y1, z1, lowerBound));
                //////////////////////////////////////////////////////////////////////////
                vertices.push_back(Vector3(y1, z1, upperBound));
                vertices.push_back(Vector3(y0, z1, upperBound));
                vertices.push_back(Vector3(y0, z0, upperBound));
                vertices.push_back(Vector3(y0, z0, upperBound));
                vertices.push_back(Vector3(y1, z0, upperBound));
                vertices.push_back(Vector3(y1, z1, upperBound));
                for (int l = 0; l < 36; ++l)
                {
                    indices.push_back(ii); ++ii;
                }
            }
        }

        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" }
		};

        //for (const Vector3& ver : vertices)
        //{
        //    AActor* actor = WorldActorCache::SpawnActor<AActor>();
        //    actor->AddActorComponent(new ULightComponent());
        //    actor->SetActorLocation(ver * Vector3(200.0f, 20.0f, 200.0f));
        //    actor->SetActorScale({0.5f, 0.5f, 0.5f});
        //}

        std::vector<Vector3> normals;
        const std::vector<std::pair<float, float>> texCoords;
        Renderer::GenerateVertexArray(GetVertexArrayName(), vertices, normals, texCoords, indices, Layout);
    }
}