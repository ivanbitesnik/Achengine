#include "Achenginepch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer2D.h"
#include "Achengine/Renderer/EditorCamera.h"
#include "Achengine/Actor/Actor.h"
#include "Achengine/Actor/MeshDrawable.h"

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
			//ACHENGINE_CORE_WARN("Shader was already added! Shader name: {0}", ShaderName);
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
		VertexBuffer* VertexBuffer = VertexBuffer::Create(VertexCoords.size(), &VertexCoords[0]);
		VertexBuffer->SetLayout(BufferLayout);
		NewVertexArray->AddVertexBuffer(VertexBuffer);
		
		s_RenderData->VertexArrays.insert({VertexArrayName, NewVertexArray});
		return NewVertexArray;
	}

	template<unsigned int N>
	void Renderer::AddIndexBufferToArray(const std::string& VertexArrayName, uint32_t (&Indices)[N])
	{
		IndexBuffer* squareIndexBuffer = IndexBuffer::Create(sizeof(Indices) / sizeof(uint32_t), Indices);
		VertexArray* VA = GetVertexArray(VertexArrayName);
		if (!VA)
		{
			return;
		}

		VA->SetIndexBuffer(squareIndexBuffer);
	}

	void Renderer::AddIndexBufferToArray(const std::string& VertexArrayName, const std::vector<uint32_t>& Indices)
	{
		IndexBuffer* squareIndexBuffer = IndexBuffer::Create(Indices.size() / sizeof(uint32_t), &Indices[0]);
		VertexArray* VA = GetVertexArray(VertexArrayName);
		if (!VA)
		{
			return;
		}

		VA->SetIndexBuffer(squareIndexBuffer);
	}

	VertexArray* Renderer::GetVertexArray(const std::string& VertexArrayName)
	{
		if (s_RenderData->VertexArrays.count(VertexArrayName))
		{
			return s_RenderData->VertexArrays.at(VertexArrayName);
		}

		return nullptr;
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
	}

	void Renderer::EndScene()
	{
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
		VertexArray* VertexArrayToDraw = GetVertexArray(VertexArrayName);
		VertexArrayToDraw->Bind();
		RenderCommand::DrawIndexed(VertexArrayToDraw);
	}

	void Renderer::DrawMesh(UMesh* Mesh, const std::string& DrawableID)
	{
		const std::string& ShaderName = Mesh->GetShaderName();
		Shader* shader = s_RenderData->GetShader(ShaderName);
		glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
		if (Mesh->GetTexture())
		{
			Mesh->GetTexture()->Bind();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if (shader) {
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

		DrawVertexArray(DrawableID);
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