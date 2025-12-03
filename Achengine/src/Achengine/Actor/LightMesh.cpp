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

		Initialize();
	}

    void ULightMesh::DrawMesh()
    {
		// TODO: remove awful dep
		UStaticMesh* temp = new UStaticMesh();

		const std::string& StaticMeshShaderName = GetObjectNameFromFilePath(temp->m_ShaderPath);
		Renderer::SetShaderUniform(StaticMeshShaderName, "u_Light.ambient", GetLightSource()->ambient);
		Renderer::SetShaderUniform(StaticMeshShaderName, "u_Light.diffuse", GetLightSource()->diffuse);
		Renderer::SetShaderUniform(StaticMeshShaderName, "u_Light.specular", GetLightSource()->specular);
		delete temp;
		
		const std::string& ShaderName = GetObjectNameFromFilePath(m_ShaderPath);
		Renderer::SetShaderUniform(ShaderName, "u_Transform", GetOwner()->GetActorTransform());
		Renderer::SetShaderUniform(ShaderName, "u_Light.color", GetLightSource()->color);
		Renderer::SetShaderUniform(ShaderName, "u_Light.ambient", GetLightSource()->ambient);
		Renderer::SetShaderUniform(ShaderName, "u_Light.diffuse", GetLightSource()->diffuse);
		Renderer::SetShaderUniform(ShaderName, "u_Light.specular", GetLightSource()->specular);

		//TODO: Remove hardcoded string
		Renderer::DrawVertexArray("LightSourceVertexArray");
    }
}