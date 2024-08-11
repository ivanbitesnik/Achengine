#include "Achenginepch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer2D.h"
#include "EditorCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Achengine
{
	struct RendererStorage
	{
		glm::mat4 ViewProjectionMatrix;
		Texture2D* WhiteTexture;
		VertexArray* QuadVertexArray;
		VertexArray* CubeVertexArray;
		VertexArray* LightSourceVertexArray;
		Shader* TextureShader;
		Shader* LightSourceShader;
		Shader* CubeShader;
	};

	static RendererStorage* s_RenderData;

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();

		s_RenderData = new RendererStorage;

		// Texture shader /////////////////////////////////////////////////
		s_RenderData->QuadVertexArray = VertexArray::Create();

		int arraySize;
		float* quadVertices = CreateQuadVertexArray(arraySize);
		VertexBuffer* squareVertexBuffer = VertexBuffer::Create(arraySize, quadVertices);
		squareVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_RenderData->QuadVertexArray->AddVertexBuffer(squareVertexBuffer);

		unsigned int* squareIndices = CreateQuadIndexArray(arraySize);
		IndexBuffer* squareIndexBuffer = IndexBuffer::Create(arraySize / sizeof(uint32_t), squareIndices);
		s_RenderData->QuadVertexArray->SetIndexBuffer(squareIndexBuffer);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		s_RenderData->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetInt("u_Texture", 0);

		// Cube shader /////////////////////////////////////////////////////
		s_RenderData->CubeVertexArray = VertexArray::Create();
		float* cubeVertices = CreateCubeVertexArray(arraySize);

		VertexBuffer* cubeVertexBuffer = VertexBuffer::Create(arraySize, cubeVertices);
		cubeVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});
		s_RenderData->CubeVertexArray->AddVertexBuffer(cubeVertexBuffer);

		float* normalsVertices = CreateNormalsVertexArray(arraySize);
		VertexBuffer* normalsVertexBuffer = VertexBuffer::Create(arraySize, normalsVertices);
		normalsVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Normal" }
		});
		s_RenderData->CubeVertexArray->AddVertexBuffer(normalsVertexBuffer);

		unsigned int* cubeIndices = CreateCubeIndexArray(arraySize);
		IndexBuffer* cubeIndexBuffer = IndexBuffer::Create(arraySize / sizeof(uint32_t), cubeIndices);
		s_RenderData->CubeVertexArray->SetIndexBuffer(cubeIndexBuffer);

		s_RenderData->CubeShader = Shader::Create("assets/shaders/Cube.glsl");
		s_RenderData->CubeShader->Bind();

		// Lighting shader /////////////////////////////////////////////////
		s_RenderData->LightSourceVertexArray = VertexArray::Create();
		float* lightSourceVertices = CreateCubeVertexArray(arraySize);
		
		VertexBuffer* lightSourceVertexBuffer = VertexBuffer::Create(arraySize, lightSourceVertices);
		lightSourceVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});
		s_RenderData->LightSourceVertexArray->AddVertexBuffer(lightSourceVertexBuffer);
		
		unsigned int* lightSourceIndices = CreateCubeIndexArray(arraySize);
		IndexBuffer* lightSourceIndexBuffer = IndexBuffer::Create(arraySize / sizeof(uint32_t), lightSourceIndices);
		s_RenderData->LightSourceVertexArray->SetIndexBuffer(lightSourceIndexBuffer);

		s_RenderData->LightSourceShader = Shader::Create("assets/shaders/LightSource.glsl");
		s_RenderData->LightSourceShader->Bind();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
		delete s_RenderData;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(Camera* camera)
	{
		EditorCamera* SceneCamera = (EditorCamera*)camera;
		s_RenderData->ViewProjectionMatrix = (SceneCamera->GetViewProjection() * SceneCamera->GetViewMatrix());
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetMat4("u_ViewProjection", s_RenderData->ViewProjectionMatrix);

		s_RenderData->CubeShader->Bind();
		s_RenderData->CubeShader->SetMat4("u_ViewProjection", s_RenderData->ViewProjectionMatrix);

		s_RenderData->LightSourceShader->Bind();
		s_RenderData->LightSourceShader->SetMat4("u_ViewProjection", s_RenderData->ViewProjectionMatrix);
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const VertexArray* vertexArray, Shader* shader, const glm::mat4 transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", s_RenderData->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const float angle, const glm::vec3& rot)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color, angle, rot);
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const float angle, const glm::vec3& rot)
	{
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetFloat4("u_Color", color);
		s_RenderData->WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, 1.0f });
		s_RenderData->TextureShader->SetMat4("u_Transform", transform);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture2D* texture, const float angle, const glm::vec3& rot, const glm::vec4& tint)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, angle, rot, tint);
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D* texture, const float angle, const glm::vec3& rot, const glm::vec4& tint)
	{
		s_RenderData->TextureShader->Bind();
		texture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, 1.0f });
		s_RenderData->TextureShader->SetMat4("u_Transform", transform);
		s_RenderData->TextureShader->SetFloat4("u_Color", tint);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}

	void Renderer::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, const float angle, const glm::vec3& rot)
	{
		s_RenderData->CubeShader->Bind();
		s_RenderData->CubeShader->SetFloat3("u_LightPosition", { 0.0f, 0.0f, 0.0f });

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, size.z });
		s_RenderData->CubeShader->SetMat4("u_Transform", transform);
		s_RenderData->CubeShader->SetFloat3("u_ObjectColor", color);
		s_RenderData->CubeShader->SetFloat3("u_LightColor", { 1.0f, 1.0f, 1.0f });

		s_RenderData->CubeVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->CubeVertexArray);
	}

	void Renderer::DrawLight(const glm::vec3& position, const glm::vec3& size, const float angle, const glm::vec3& rot)
	{
		s_RenderData->LightSourceShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, size.z });
		s_RenderData->LightSourceShader->SetMat4("u_Transform", transform);

		s_RenderData->LightSourceVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->LightSourceVertexArray);
	}
}