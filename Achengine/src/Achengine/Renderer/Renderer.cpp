#include "Achenginepch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer2D.h"
#include "Achengine/Renderer/EditorCamera.h"
#include "Achengine/Actor/Actor.h"

#include <glm/gtc/matrix_transform.hpp>

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
		IndexBuffer* squareIndexBuffer = IndexBuffer::Create(sizeof(quadIndexArray) / sizeof(uint32_t), quadIndexArray);
		QuadVertexArray->SetIndexBuffer(squareIndexBuffer);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		Shader* TextureShader = Renderer::AddShader(TextureShaderPath);
		Renderer::SetShaderUniform(TextureShader->GetName(), "u_Texture", 0);
		
		// Basic Water shader //////////////////////////////////////////////
		BufferLayout BasicWaterBufferLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};
		AddVertexArray("BasicWaterVertexArray", cubeWithNormalsVertexArray, BasicWaterBufferLayout);

		// Cube shader /////////////////////////////////////////////////////
		BufferLayout CubeBufferLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoords"}
		};
		AddVertexArray("CubeVertexArray", texturedCubeWithNormalsVertexArray, CubeBufferLayout);

		// Lighting shader /////////////////////////////////////////////////
		BufferLayout LightSourceBufferLayout = {
			{ ShaderDataType::Float3, "a_Position" }
		};
		AddVertexArray("LightSourceVertexArray", cubeVertexArray, LightSourceBufferLayout);
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
	VertexArray* Renderer::AddVertexArray(const std::string& VertexArrayName, const float (&VertexCoords)[N], const BufferLayout& BufferLayout)
	{
		VertexArray* NewVertexArray = VertexArray::Create();
		VertexBuffer* VertexBuffer = VertexBuffer::Create(sizeof(VertexCoords), VertexCoords);
		VertexBuffer->SetLayout(BufferLayout);
		NewVertexArray->AddVertexBuffer(VertexBuffer);
		
		s_RenderData->VertexArrays.insert({VertexArrayName, NewVertexArray});
		return NewVertexArray;
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
		const glm::vec3& CameraPosition = Camera->GetPosition();
		const glm::mat4& ViewProjectionMatrix = (Camera->GetViewProjection() * Camera->GetViewMatrix());

		for (Shader* shader : s_RenderData->Shaders)
		{
			Renderer::SetShaderUniform(shader->GetName(), "u_ViewProjection", ViewProjectionMatrix);
			Renderer::SetShaderUniform(shader->GetName(), "u_ViewPosition", CameraPosition);
		}
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