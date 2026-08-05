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
		m_Bounds.LocalCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Bounds.LocalExtents = glm::vec3(1.0f, 1.0f, 1.0f);
		m_Bounds.LocalSphereRadius = glm::length(m_Bounds.LocalExtents);
		m_Bounds.IsValid = true;
		Initialize();
	}

	void ULightMesh::SetUniforms()
	{
		const std::string& ShaderName = GetObjectNameFromFilePath(m_ShaderPath);
		Renderer::SetShaderUniform(ShaderName, "u_Light.color", GetLightSource()->color);
		Renderer::SetShaderUniform(ShaderName, "u_Light.ambient", GetLightSource()->ambient);
		Renderer::SetShaderUniform(ShaderName, "u_Light.diffuse", GetLightSource()->diffuse);
		Renderer::SetShaderUniform(ShaderName, "u_Light.specular", GetLightSource()->specular);
	}

	void ULightMesh::SubmitLighting()
	{
		if (!GetOwner() || !GetLightSource())
		{
			return;
		}

		Renderer::AddSceneLight(
			GetOwner()->GetActorLocation(),
			GetLightSource()->ambient,
			GetLightSource()->diffuse,
			GetLightSource()->specular,
			GetLightSource()->constant,
			GetLightSource()->linear,
			GetLightSource()->quadratic
		);
	}

	void ULightMesh::GenerateVertexArray()
	{
		static float cubeWithNormalsVertexArray[6 * 6 * 6] = {
			// positions          // normals
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 
			1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,  
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,  
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,  
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 

			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,

			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 
			1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 
			1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 

			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
			1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
			
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    	};

        std::vector<Vector3> vertices;
        std::vector<Vector3> normals;
        Vector3 currentVertex;
		constexpr uint32_t stride = 6;
		const uint32_t floatCount = sizeof(cubeWithNormalsVertexArray) / sizeof(cubeWithNormalsVertexArray[0]);
		for (uint32_t i = 0; i + (stride - 1) < floatCount; i += stride)
        {
			currentVertex.X = cubeWithNormalsVertexArray[i];
            currentVertex.Y = cubeWithNormalsVertexArray[i+1];
            currentVertex.Z = cubeWithNormalsVertexArray[i+2];
            vertices.push_back(currentVertex);
            
            currentVertex.X = cubeWithNormalsVertexArray[i+3];
            currentVertex.Y = cubeWithNormalsVertexArray[i+4];
            currentVertex.Z = cubeWithNormalsVertexArray[i+5];
            normals.push_back(currentVertex);
        }
		
        std::vector<uint32_t> indices;
		for (uint32_t j = 0; j < vertices.size(); ++j)
        {
			indices.push_back(j);
        }
		
        BufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};
		
		std::vector<std::pair<float, float>> texCoords;
		Renderer::GenerateVertexArray(GetVertexArrayName(), vertices, normals, texCoords, indices, Layout);
	}

	void ULightMesh::DrawMesh()
	{
		const std::string& ShaderName = GetObjectNameFromFilePath(m_ShaderPath);
		Renderer::SetShaderUniform(ShaderName, "u_Light.color", GetLightSource()->color);
		Renderer::SetShaderUniform(ShaderName, "u_Light.ambient", GetLightSource()->ambient);
		Renderer::SetShaderUniform(ShaderName, "u_Light.diffuse", GetLightSource()->diffuse);
		Renderer::SetShaderUniform(ShaderName, "u_Light.specular", GetLightSource()->specular);
	
		UMesh::DrawMesh();
	}
}
