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
		s_RenderData->QuadVertexArray = VertexArray::Create();

		VertexBuffer* quadVertexBuffer = VertexBuffer::Create(sizeof(quadVertexArray), quadVertexArray);
		quadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_RenderData->QuadVertexArray->AddVertexBuffer(quadVertexBuffer);

		IndexBuffer* squareIndexBuffer = IndexBuffer::Create(sizeof(quadIndexArray) / sizeof(uint32_t), quadIndexArray);
		s_RenderData->QuadVertexArray->SetIndexBuffer(squareIndexBuffer);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		Renderer::AddShader("TextureShader", TextureShaderPath);
		Renderer::SetShaderUniform("TextureShader", "u_Texture", 0);
		
		// Basic Water shader //////////////////////////////////////////////
		s_RenderData->BasicWaterVertexArray = VertexArray::Create();
		VertexBuffer* waterVertexBuffer = VertexBuffer::Create(sizeof(cubeWithNormalsVertexArray), cubeWithNormalsVertexArray);
		waterVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		});
		s_RenderData->BasicWaterVertexArray->AddVertexBuffer(waterVertexBuffer);

		// Cube shader /////////////////////////////////////////////////////
		s_RenderData->CubeVertexArray = VertexArray::Create();
		VertexBuffer* cubeVertexBuffer = VertexBuffer::Create(sizeof(texturedCubeWithNormalsVertexArray), texturedCubeWithNormalsVertexArray);
		cubeVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoords"}
		});
		s_RenderData->CubeVertexArray->AddVertexBuffer(cubeVertexBuffer);

		// Lighting shader /////////////////////////////////////////////////
		s_RenderData->LightSourceVertexArray = VertexArray::Create();
		
		VertexBuffer* lightSourceVertexBuffer = VertexBuffer::Create(sizeof(cubeVertexArray), cubeVertexArray);
		lightSourceVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});
		s_RenderData->LightSourceVertexArray->AddVertexBuffer(lightSourceVertexBuffer);
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
		delete s_RenderData;
	}

	void Renderer::AddShader(const std::string& ShaderName, const std::string& ShaderPath)
	{
		if (s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Shader was already added! Shader name: {0}", ShaderName);
			return;
		}
		
		Shader* NewShader = Shader::Create(ShaderPath);
		s_RenderData->Shaders.insert({ShaderName, NewShader});
		NewShader->Bind();
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

		for (auto& shader : s_RenderData->Shaders)
		{
			Renderer::SetShaderUniform(shader.first, "u_ViewProjection", ViewProjectionMatrix);
			Renderer::SetShaderUniform(shader.first, "u_ViewPosition", CameraPosition);
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

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}

	void Renderer::DrawActor(AActor* ActorToDraw)
	{
		ActorToDraw->Draw(s_RenderData);
	}

	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, int value)
	{
		if (!s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		Shader* shader = s_RenderData->Shaders.at(ShaderName);

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetInt(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, float value)
	{
		if (!s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		Shader* shader = s_RenderData->Shaders.at(ShaderName);

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec2& value)
	{
		if (!s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		Shader* shader = s_RenderData->Shaders.at(ShaderName);

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat2(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec3& value)
	{
		if (!s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		Shader* shader = s_RenderData->Shaders.at(ShaderName);

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat3(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::vec4& value)
	{
		if (!s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		Shader* shader = s_RenderData->Shaders.at(ShaderName);

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetFloat4(UniformName, value);
		}
	}
	void Renderer::SetShaderUniform(const std::string& ShaderName, const std::string& UniformName, const glm::mat4& value)
	{
		if (!s_RenderData->Shaders.count(ShaderName))
		{
			ACHENGINE_CORE_WARN("Tried to set shader uniform to an invalid shader! Shader name: {0}, uniform name: {1}", ShaderName, UniformName);
			return;
		}

		Shader* shader = s_RenderData->Shaders.at(ShaderName);

		shader->Bind();
		if (shader->HasUniform(UniformName))
		{
			shader->SetMat4(UniformName, value);
		}
	}
}