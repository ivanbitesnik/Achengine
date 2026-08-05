#include "Achenginepch.h"
#include "StaticMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"
#include "Achengine/Renderer/EditorCamera.h"

namespace Achengine
{

    UStaticMesh::UStaticMesh()
    {
// Shader path should eventually be set somewhere else _within_ the engine
#ifdef ACHENGINE_PLATFORM_LINUX
	    m_ShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/Cube.glsl";
#else
	    m_ShaderPath = "assets/shaders/Cube.glsl";
#endif
        m_Bounds.LocalCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        m_Bounds.LocalExtents = glm::vec3(1.0f, 1.0f, 1.0f);
        m_Bounds.LocalSphereRadius = glm::length(m_Bounds.LocalExtents);
        m_Bounds.IsValid = true;
        Initialize();
    }

    void UStaticMesh::SetUniforms()
    {
        const std::string& ShaderName = GetObjectNameFromFilePath(m_ShaderPath);

        if (FMeshMaterial* Material = GetMaterial())
        {
            Renderer::SetShaderUniform(ShaderName, "u_Material.color", Material->color);
            Renderer::SetShaderUniform(ShaderName, "u_Material.ambient", Material->ambient);
            Renderer::SetShaderUniform(ShaderName, "u_Material.diffuse", Material->diffuse);
            Renderer::SetShaderUniform(ShaderName, "u_Material.specular", Material->specular);
            Renderer::SetShaderUniform(ShaderName, "u_Material.shininess", Material->shininess);
            Renderer::SetShaderUniform(ShaderName, "u_Material.normal", 2);
            Renderer::SetShaderUniform(ShaderName, "u_HasDiffuseMap", 0);
            Renderer::SetShaderUniform(ShaderName, "u_HasSpecularMap", 0);
            Renderer::SetShaderUniform(ShaderName, "u_HasNormalMap", 0);
        }
        else if (Texture* texture = GetTexture())
        {
            Renderer::SetShaderUniform(ShaderName, "u_Material.diffuse", 0);
            Renderer::SetShaderUniform(ShaderName, "u_Material.specular", 1);
            Renderer::SetShaderUniform(ShaderName, "u_Material.normal", 2);
            Renderer::SetShaderUniform(ShaderName, "u_Material.shininess", 256.0f);
		    Renderer::SetShaderUniform(ShaderName, "u_HasDiffuseMap", 1);
		    Renderer::SetShaderUniform(ShaderName, "u_HasSpecularMap", GetSpecular() ? 1 : 0);
		    Renderer::SetShaderUniform(ShaderName, "u_HasNormalMap", 0);
		    texture->Bind();
            if (Texture* Specular = GetSpecular())
            {
                texture->BindSpecularMap(Specular);
            }
        }
    }

    void UStaticMesh::GenerateVertexArray()
    {
        static float texturedCubeWithNormalsVertexArray[8 * 6 * 6] = 
        {
            // positions          // normals           // texture coords
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
            
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
        };
        
        std::vector<Vector3> vertices;
        std::vector<Vector3> normals;
        std::vector<std::pair<float, float>> texCoords;
        Vector3 currentVertex;
        std::pair<float, float> currentTexCoord;
        constexpr uint32_t stride = 8;
        const uint32_t floatCount = sizeof(texturedCubeWithNormalsVertexArray) / sizeof(texturedCubeWithNormalsVertexArray[0]);
        for (uint32_t i = 0; i + (stride - 1) < floatCount; i += stride)
        {
            currentVertex.X = texturedCubeWithNormalsVertexArray[i];
            currentVertex.Y = texturedCubeWithNormalsVertexArray[i+1];
            currentVertex.Z = texturedCubeWithNormalsVertexArray[i+2];
            vertices.push_back(currentVertex);
            
            currentVertex.X = texturedCubeWithNormalsVertexArray[i+3];
            currentVertex.Y = texturedCubeWithNormalsVertexArray[i+4];
            currentVertex.Z = texturedCubeWithNormalsVertexArray[i+5];
            normals.push_back(currentVertex);
            
            currentTexCoord.first = texturedCubeWithNormalsVertexArray[i+6];
            currentTexCoord.second = texturedCubeWithNormalsVertexArray[i+7];
            texCoords.push_back(currentTexCoord);
        }

        std::vector<uint32_t> indices;
        for (uint32_t j = 0; j < vertices.size(); ++j)
        {
            indices.push_back(j);
        }

        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoords"}
		};

        Renderer::GenerateVertexArray(GetVertexArrayName(), vertices, normals, texCoords, indices, Layout);
    }
}