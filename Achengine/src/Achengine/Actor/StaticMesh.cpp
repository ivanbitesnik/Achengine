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
        }
        else if (Texture* texture = GetTexture())
        {
            Renderer::SetShaderUniform(ShaderName, "u_Material.diffuse", 0);
            Renderer::SetShaderUniform(ShaderName, "u_Material.specular", 1);
            Renderer::SetShaderUniform(ShaderName, "u_Material.shininess", 256.0f);
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
        
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<std::pair<float, float>> texCoords;
        glm::vec3 currentVertex;
        std::pair<float, float> currentTexCoord;
        for (int i = 0; i < sizeof(texturedCubeWithNormalsVertexArray); i += 8)
        {
            currentVertex.x = texturedCubeWithNormalsVertexArray[i];
            currentVertex.y = texturedCubeWithNormalsVertexArray[i+1];
            currentVertex.z = texturedCubeWithNormalsVertexArray[i+2];
            vertices.push_back(currentVertex);
            
            currentVertex.x = texturedCubeWithNormalsVertexArray[i+3];
            currentVertex.y = texturedCubeWithNormalsVertexArray[i+4];
            currentVertex.z = texturedCubeWithNormalsVertexArray[i+5];
            normals.push_back(currentVertex);
            
            currentTexCoord.first = texturedCubeWithNormalsVertexArray[i+6];
            currentTexCoord.second = texturedCubeWithNormalsVertexArray[i+7];
            texCoords.push_back(currentTexCoord);
        }

        std::vector<uint32_t> indices;
        for (int j = 0; j < vertices.size(); ++j) 
        {
            indices.push_back(j);
        }

        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoords"}
		};

        Renderer::GenerateVertexArray(GetShaderName(), vertices, normals, texCoords, indices, Layout);
    }
}