#include "Achenginepch.h"
#include "LightMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Achengine
{
	ULightMesh::ULightMesh()
	{
		m_LightSource = new FLightSource();

#ifdef ACHENGINE_PLATFORM_LINUX
		m_ShaderPath =  "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/LightSource.glsl";
#else
		m_ShaderPath = "assets/shaders/LightSource.glsl";
#endif
		m_ShaderName = "LightSourceShader";

		Initialize();
	}

    void ULightMesh::DrawMesh(RendererStorage* RenderData)
    {
		// TODO: remove access of another shader name
		Renderer::SetShaderUniform("StaticMeshShader", "u_Light.ambient", GetLightSource()->ambient);
		Renderer::SetShaderUniform("StaticMeshShader", "u_Light.diffuse", GetLightSource()->diffuse);
		Renderer::SetShaderUniform("StaticMeshShader", "u_Light.specular", GetLightSource()->specular);

		Renderer::SetShaderUniform(m_ShaderName, "u_Transform", GetOwner()->GetActorTransform());
		Renderer::SetShaderUniform(m_ShaderName, "u_Light.color", GetLightSource()->color);
		Renderer::SetShaderUniform(m_ShaderName, "u_Light.ambient", GetLightSource()->ambient);
		Renderer::SetShaderUniform(m_ShaderName, "u_Light.diffuse", GetLightSource()->diffuse);
		Renderer::SetShaderUniform(m_ShaderName, "u_Light.specular", GetLightSource()->specular);

		RenderData->LightSourceVertexArray->Bind();
		RenderCommand::DrawIndexed(RenderData->LightSourceVertexArray);
    }
}