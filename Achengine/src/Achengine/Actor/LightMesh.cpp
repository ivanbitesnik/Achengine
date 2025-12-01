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
	}

    void ULightMesh::DrawMesh(RendererStorage* RenderData)
    {
        RenderData->CubeShader->Bind();
		RenderData->CubeShader->SetFloat3("u_Light.ambient", GetLightSource()->ambient);
		RenderData->CubeShader->SetFloat3("u_Light.diffuse", GetLightSource()->diffuse);
		RenderData->CubeShader->SetFloat3("u_Light.specular", GetLightSource()->specular);

		RenderData->LightSourceShader->Bind();
		RenderData->LightSourceShader->SetMat4("u_Transform", GetOwner()->GetActorTransform());
		RenderData->LightSourceShader->SetFloat3("u_Light.color", GetLightSource()->color);
		RenderData->LightSourceShader->SetFloat3("u_Light.ambient", GetLightSource()->ambient);
		RenderData->LightSourceShader->SetFloat3("u_Light.diffuse", GetLightSource()->diffuse);
		RenderData->LightSourceShader->SetFloat3("u_Light.specular", GetLightSource()->specular);

		RenderData->LightSourceVertexArray->Bind();
		RenderCommand::DrawIndexed(RenderData->LightSourceVertexArray);
    }
}