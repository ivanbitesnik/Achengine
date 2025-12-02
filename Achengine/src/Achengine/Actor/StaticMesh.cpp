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
        m_ShaderName = "StaticMeshShader";
        Initialize();
    }

    void UStaticMesh::Initialize()
    {
        Renderer::AddShader(m_ShaderName, m_ShaderPath);
    }

    void UStaticMesh::DrawMesh(RendererStorage* RenderData)
    {
		Renderer::SetShaderUniform(m_ShaderName, "u_Transform", GetOwner()->GetActorTransform());

        if (FMeshMaterial* Material = GetMaterial())
        {
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.color", Material->color);
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.ambient", Material->ambient);
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.diffuse", Material->diffuse);
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.specular", Material->specular);
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.shininess", Material->shininess);
        }
        else if (Texture* texture = GetTexture())
        {
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.diffuse", 0);
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.specular", 1);
            Renderer::SetShaderUniform(m_ShaderName, "u_Material.shininess", 256.0f);
		    texture->Bind();
            if (Texture* Specular = GetSpecular())
            {
                texture->BindSpecularMap(Specular);
            }
        }

		RenderData->CubeVertexArray->Bind();
		RenderCommand::DrawIndexed(RenderData->CubeVertexArray);
    }
}