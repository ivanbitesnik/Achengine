#include "Achenginepch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer2D.h"
#include "Achengine/Renderer/EditorCamera.h"
#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/Mesh.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

namespace Achengine
{
	static RendererStorage* s_RenderData;

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();

		s_RenderData = new RendererStorage;

		// Texture shader /////////////////////////////////////////////////
		BufferLayout QuadBufferLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		VertexArray* QuadVertexArray = AddVertexArray("QuadVertexArray", quadVertexArray, QuadBufferLayout);
		AddIndexBufferToArray("QuadVertexArray", quadIndexArray);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		Shader* TextureShader = Renderer::AddShader(TextureShaderPath);
		Renderer::SetShaderUniform(TextureShader->GetName(), "u_Texture", 0);
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
		delete s_RenderData;
	}

	Shader* Renderer::AddShader(const std::string& ShaderPath)
	{
		const std::string& ShaderName = GetObjectNameFromFilePath(ShaderPath);
		if (s_RenderData->GetShader(ShaderName))
		{
			return nullptr;
		}
		
		Shader* NewShader = Shader::Create(ShaderPath);
		s_RenderData->Shaders.push_back(NewShader);
		NewShader->Bind();
		return NewShader;
	}

	template<unsigned int N>
	VertexArray* Renderer::AddVertexArray(const std::string& VertexArrayName, float (&VertexCoords)[N], const BufferLayout& BufferLayout)
	{
		VertexArray* NewVertexArray = VertexArray::Create();
		VertexBuffer* VertexBuffer = VertexBuffer::Create(sizeof(VertexCoords), VertexCoords);
		VertexBuffer->SetLayout(BufferLayout);
		NewVertexArray->AddVertexBuffer(VertexBuffer);
		
		s_RenderData->VertexArrays.insert({VertexArrayName, NewVertexArray});
		return NewVertexArray;
	}

	VertexArray* Renderer::AddVertexArray(const std::string& VertexArrayName, const std::vector<float>& VertexCoords, const BufferLayout& BufferLayout)
	{
		VertexArray* NewVertexArray = VertexArray::Create();
		VertexBuffer* VertexBuffer = VertexBuffer::Create((uint32_t)(VertexCoords.size() * sizeof(float)), VertexCoords.data());
		VertexBuffer->SetLayout(BufferLayout);
		NewVertexArray->AddVertexBuffer(VertexBuffer);
		
		s_RenderData->VertexArrays.insert({VertexArrayName, NewVertexArray});
		return NewVertexArray;
	}

	template<unsigned int N>
	void Renderer::AddIndexBufferToArray(const std::string& VertexArrayName, uint32_t (&Indices)[N])
	{
		IndexBuffer* indexBuffer = IndexBuffer::Create(N, Indices);
		VertexArray* VA = GetVertexArray(VertexArrayName);
		if (!VA)
		{
			return;
		}

		VA->SetIndexBuffer(indexBuffer);
	}

	void Renderer::AddIndexBufferToArray(const std::string& VertexArrayName, const std::vector<uint32_t>& Indices)
	{
		IndexBuffer* indexBuffer = IndexBuffer::Create(Indices.size(), Indices.data());
		VertexArray* VA = GetVertexArray(VertexArrayName);
		if (!VA)
		{
			return;
		}

		VA->SetIndexBuffer(indexBuffer);
	}

	VertexArray* Renderer::GetVertexArray(const std::string& VertexArrayName)
	{
		if (s_RenderData->VertexArrays.count(VertexArrayName))
		{
			return s_RenderData->VertexArrays.at(VertexArrayName);
		}

		return nullptr;
	}

	void Renderer::GenerateNormals(const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices, std::vector<Vector3>& normals)
    {
        normals.resize(indices.size());
        for (int i = 0; i < indices.size(); i += 3) 
		{
            uint32_t tri[3] = { indices[i], indices[i + 1], indices[i + 2] };
            glm::vec3 a = vertices[tri[0]], 
            b = vertices[tri[1]],
            c = vertices[tri[2]];
            glm::vec3 ab = b - a, ac = c - a;
            glm::vec3 n = glm::normalize(glm::cross(ab, ac));
            for (int j = 0; j < 3; ++j)
			{
				normals[tri[j]] = n;
			}
        }

        for (int i = 0; i < normals.size(); ++i)
		{
			glm::vec3 n = normals[i];
            normals[i] = glm::normalize(n);
		}
    }

    void Renderer::GenerateVertexArray(const std::string& VertexId, const std::vector<Vector3>& vertices, std::vector<Vector3>& normals,
		 const std::vector<std::pair<float, float>>& texCoords, const std::vector<uint32_t>& indices, const BufferLayout& Layout)
    {
		if (normals.size() == 0)
		{
			//GenerateNormals(vertices, indices, normals);
		}
        std::vector<float> vertexArray;
        for (int i = 0; i < vertices.size(); ++i)
        {
            vertexArray.push_back(vertices[i].X);
            vertexArray.push_back(vertices[i].Y);
            vertexArray.push_back(vertices[i].Z);
            if (normals.size() > i)
            {
                vertexArray.push_back(normals[i].X);
                vertexArray.push_back(normals[i].Y);
                vertexArray.push_back(normals[i].Z);
            }
            if (texCoords.size() > i)
            {
                vertexArray.push_back(texCoords[i].first);
                vertexArray.push_back(texCoords[i].second);
            }
        }

        Renderer::AddVertexArray(VertexId, vertexArray, Layout);
        if (indices.size() > 0)
        {
            Renderer::AddIndexBufferToArray(VertexId, indices);
        }
    }

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(Camera* camera)
	{
		EditorCamera* Camera = (EditorCamera*)camera;
		s_RenderData->CameraPosition = Camera->GetPosition();
		s_RenderData->ViewProjectionMatrix = (Camera->GetViewProjection() * Camera->GetViewMatrix());
		ClearSceneLights();

		if (Achengine::WorldActorCache* Cache = Achengine::WorldActorCache::Get())
		{
			for (Achengine::AActor* Actor : Cache->GetActorCache())
			{
				if (UMesh* Mesh = Actor->GetMesh())
				{
					Mesh->SubmitLighting();
				}
			}

			for (Achengine::AActor* Actor : Cache->GetActorCache())
			{
				Actor->Draw();
			}
		}
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::AddSceneLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular,
		float constant, float linear, float quadratic)
	{
		if (s_RenderData->SceneLights.size() >= MaxSceneLights)
		{
			return;
		}

		FSceneLight light;
		light.Position = position;
		light.Ambient = ambient;
		light.Diffuse = diffuse;
		light.Specular = specular;
		light.Constant = constant;
		light.Linear = linear;
		light.Quadratic = quadratic;
		s_RenderData->SceneLights.push_back(light);
	}

	void Renderer::ClearSceneLights()
	{
		s_RenderData->SceneLights.clear();
	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture2D* texture, const float angle, const glm::vec3& rot, const glm::vec4& tint)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, angle, rot, tint);
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D* texture, const float angle, const glm::vec3& rot, const glm::vec4& tint)
	{
		Renderer::SetShaderUniform("TextureShader", "u_Color", tint);
		if (texture)
		{
			texture->Bind();
		}
		else
		{
			s_RenderData->WhiteTexture->Bind();
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, 1.0f });
		Renderer::SetShaderUniform("TextureShader", "u_Transform", transform);

		VertexArray* QuadVertexArray = GetVertexArray("QuadVertexArray");
		QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(QuadVertexArray);
	}

	void Renderer::DrawVertexArray(const std::string& VertexArrayName)
	{
		if (VertexArray* VertexArrayToDraw = GetVertexArray(VertexArrayName))
		{
			VertexArrayToDraw->Bind();
			RenderCommand::DrawIndexed(VertexArrayToDraw);
		}
	}

	void Renderer::DrawMesh(UMesh* Mesh)
	{
		const std::string& ShaderName = Mesh->GetShaderName();
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (Mesh->GetTexture())
		{
			Mesh->GetTexture()->Bind();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		
		if (shader) {
			const int lightCount = (int)s_RenderData->SceneLights.size();
			SetShaderUniform(ShaderName, "u_LightCount", lightCount);
			for (int i = 0; i < lightCount; ++i)
			{
				const FSceneLight& light = s_RenderData->SceneLights[i];
				SetShaderUniform(ShaderName, format("u_Lights[%d].position", i), light.Position);
				SetShaderUniform(ShaderName, format("u_Lights[%d].ambient", i), light.Ambient);
				SetShaderUniform(ShaderName, format("u_Lights[%d].diffuse", i), light.Diffuse);
				SetShaderUniform(ShaderName, format("u_Lights[%d].specular", i), light.Specular);
				SetShaderUniform(ShaderName, format("u_Lights[%d].constant", i), light.Constant);
				SetShaderUniform(ShaderName, format("u_Lights[%d].linear", i), light.Linear);
				SetShaderUniform(ShaderName, format("u_Lights[%d].quadratic", i), light.Quadratic);
			}

			if (shader->HasUniform("u_Time"))
			{
				SetShaderUniform(ShaderName, "u_Time", (float)getTime());
			}
			if (shader->HasUniform("u_ViewPosition"))
			{
				SetShaderUniform(ShaderName, "u_ViewPosition", s_RenderData->CameraPosition);
			}
			if (shader->HasUniform("u_Transform"))
			{
				SetShaderUniform(ShaderName, "u_Transform", Mesh->GetOwner()->GetActorTransform());
			}
			
			SetShaderUniform(ShaderName, "u_ViewProjection", s_RenderData->ViewProjectionMatrix);
			shader->Bind();
		}

		Mesh->DrawGeometry();
	}

	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, int value)
	{
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (!shader)
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetInt(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, float value)
	{
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (!shader)
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec2& value)
	{
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (!shader)
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat2(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec3& value)
	{
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (!shader)
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat3(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec4& value)
	{
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (!shader)
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat4(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::mat4& value)
	{
		Shader* shader = s_RenderData->GetShader(ShaderName);
		if (!shader)
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetMat4(UniformName, value);
		}
	}
}