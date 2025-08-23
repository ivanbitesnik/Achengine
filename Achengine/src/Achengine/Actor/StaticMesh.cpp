#include "Achenginepch.h"
#include "StaticMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"
#include "Achengine/Renderer/EditorCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Achengine
{
    void UStaticMesh::DrawMesh(RendererStorage* RenderData)
    {
        RenderData->CubeShader->Bind();
        glm::vec3 scale = GetOwner()->GetActorScale();
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), GetOwner()->GetActorLocation());
		transform = glm::rotate(transform, glm::radians(GetOwner()->GetActorRotation().Angle), GetOwner()->GetActorRotation().RotationAxis);
		transform = glm::scale(transform, { scale.x, scale.y, scale.z });
		RenderData->CubeShader->SetMat4("u_Transform", transform);
		RenderData->CubeShader->SetFloat3("u_ViewPosition", RenderData->CameraPosition);

        if (FMeshMaterial* Material = GetMaterial())
        {
            RenderData->CubeShader->SetFloat3("u_Material.color", Material->color);
            RenderData->CubeShader->SetFloat3("u_Material.ambient", Material->ambient);
            RenderData->CubeShader->SetFloat3("u_Material.diffuse", Material->diffuse);
            RenderData->CubeShader->SetFloat3("u_Material.specular", Material->specular);
            RenderData->CubeShader->SetFloat("u_Material.shininess", Material->shininess);
        }
        else if (Texture* texture = GetTexture())
        {
            RenderData->CubeShader->SetInt("u_Material.diffuse", 0);
            RenderData->CubeShader->SetInt("u_Material.specular", 1);
            RenderData->CubeShader->SetFloat("u_Material.shininess", 256.0f);
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