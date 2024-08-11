#include "Achenginepch.h"
#include "Renderer2D.h"

#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Achengine
{
	struct Renderer2DStorage
	{
		VertexArray* QuadVertexArray;
		Shader* TextureShader;
		Texture2D* WhiteTexture;
	};

	static Renderer2DStorage* s_RenderData;

	void Renderer2D::Init()
	{
		s_RenderData = new Renderer2DStorage;

		s_RenderData->QuadVertexArray = VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.75f, -0.75f, 0.0f, 0.0f, 0.0f,
			 0.75f, -0.75f, 0.0f, 1.0f, 0.0f,
			 0.75f,  0.75f, 0.0f, 1.0f, 1.0f,
			-0.75f,  0.75f, 0.0f, 0.0f, 1.0f
		};

		VertexBuffer* squareVertexBuffer;
		squareVertexBuffer = VertexBuffer::Create(sizeof(squareVertices), squareVertices);
		squareVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TextCoord" }
			});
		s_RenderData->QuadVertexArray->AddVertexBuffer(squareVertexBuffer);

		unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		IndexBuffer* squareIndexBuffer;
		squareIndexBuffer = IndexBuffer::Create(sizeof(squareIndices) / sizeof(uint32_t), squareIndices);
		s_RenderData->QuadVertexArray->SetIndexBuffer(squareIndexBuffer);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		s_RenderData->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_RenderData;
	}

	void Renderer2D::BeginScene(OrthographicCamera* camera)
	{
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetMat4("u_ViewProjection", camera->GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const float angle, const glm::vec3& rot)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color, angle, rot);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const float angle, const glm::vec3& rot)
	{
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetFloat4("u_Color", color);
		s_RenderData->WhiteTexture->Bind();

		const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), rot);
		const glm::mat4 scale = glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::mat4 transform = translation * rotation * scale;
		s_RenderData->TextureShader->SetMat4("u_Transform", transform);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture2D* texture, const float angle, const glm::vec3& rot, const glm::vec4& tint)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, angle, rot, tint);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D* texture, const float angle, const glm::vec3& rot, const glm::vec4& tint)
	{
		s_RenderData->TextureShader->Bind();
		texture->Bind();

		const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), rot);
		const glm::mat4 scale = glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		glm::mat4 transform = translation * rotation * scale;
		s_RenderData->TextureShader->SetMat4("u_Transform", transform);
		s_RenderData->TextureShader->SetFloat4("u_Color", tint);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}
}