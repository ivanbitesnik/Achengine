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
		EditorCamera* camera;
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

		VertexBuffer* squareVertexBuffer = VertexBuffer::Create(sizeof(quadVertexArray), quadVertexArray);
		squareVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_RenderData->QuadVertexArray->AddVertexBuffer(squareVertexBuffer);

		IndexBuffer* squareIndexBuffer = IndexBuffer::Create(sizeof(quadIndexArray) / sizeof(uint32_t), quadIndexArray);
		s_RenderData->QuadVertexArray->SetIndexBuffer(squareIndexBuffer);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		s_RenderData->TextureShader = Shader::Create(TextureShaderPath);
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetInt("u_Texture", 0);

		// Cube shader /////////////////////////////////////////////////////
		s_RenderData->CubeVertexArray = VertexArray::Create();
		VertexBuffer* cubeVertexBuffer = VertexBuffer::Create(sizeof(cubeWithNormalsVertexArray), cubeWithNormalsVertexArray);
		cubeVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoords"}
		});
		s_RenderData->CubeVertexArray->AddVertexBuffer(cubeVertexBuffer);

		s_RenderData->CubeShader = Shader::Create(CubeShaderPath);
		s_RenderData->CubeShader->Bind();

		// Lighting shader /////////////////////////////////////////////////
		s_RenderData->LightSourceVertexArray = VertexArray::Create();
		
		VertexBuffer* lightSourceVertexBuffer = VertexBuffer::Create(sizeof(cubeVertexArray), cubeVertexArray);
		lightSourceVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});
		s_RenderData->LightSourceVertexArray->AddVertexBuffer(lightSourceVertexBuffer);

		s_RenderData->LightSourceShader = Shader::Create(LightSourceShaderPath);
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
		s_RenderData->camera = SceneCamera;
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

	void Renderer::DrawCube(const glm::vec3& position, const glm::vec3& size, const MeshMaterial material, const float angle, const glm::vec3& rot)
	{
		s_RenderData->CubeShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, size.z });
		s_RenderData->CubeShader->SetMat4("u_Transform", transform);
		s_RenderData->CubeShader->SetFloat3("u_ViewPosition", s_RenderData->camera->GetPosition());
		s_RenderData->CubeShader->SetFloat3("u_Material.color", material.color);
		s_RenderData->CubeShader->SetFloat3("u_Material.ambient", material.ambient);
		s_RenderData->CubeShader->SetFloat3("u_Material.diffuse", material.diffuse);
		s_RenderData->CubeShader->SetFloat3("u_Material.specular", material.specular);
		s_RenderData->CubeShader->SetFloat("u_Material.shininess", material.shininess);

		s_RenderData->CubeVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->CubeVertexArray);
	}

	void Renderer::DrawCube(const glm::vec3& position, const glm::vec3& size, const Texture* texture, const Texture* specularMap, const float angle, const glm::vec3& rot)
	{
		s_RenderData->CubeShader->Bind();
		s_RenderData->CubeShader->SetInt("u_Material.diffuse", 0);
		s_RenderData->CubeShader->SetInt("u_Material.specular", 1);
		s_RenderData->CubeShader->SetFloat("u_Material.shininess", 256.0f);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, size.z });
		s_RenderData->CubeShader->SetMat4("u_Transform", transform);
		s_RenderData->CubeShader->SetFloat3("u_ViewPosition", s_RenderData->camera->GetPosition());

		texture->Bind();
		texture->BindSpecularMap(specularMap);
		s_RenderData->CubeVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->CubeVertexArray);
	}

	void Renderer::DrawLight(const LightSource lightSource, const glm::vec3& size, const float angle, const glm::vec3& rot)
	{
		s_RenderData->CubeShader->Bind();
		s_RenderData->CubeShader->SetFloat3("u_Light.position", lightSource.position);
		s_RenderData->CubeShader->SetFloat3("u_Light.ambient", lightSource.ambient);
		s_RenderData->CubeShader->SetFloat3("u_Light.diffuse", lightSource.diffuse);
		s_RenderData->CubeShader->SetFloat3("u_Light.specular", lightSource.specular);

		s_RenderData->LightSourceShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), lightSource.position);
		transform = glm::rotate(transform, glm::radians(angle), rot);
		transform = glm::scale(transform, { size.x, size.y, size.z });
		s_RenderData->LightSourceShader->SetMat4("u_Transform", transform);
		s_RenderData->LightSourceShader->SetFloat3("u_Light.color", lightSource.color);
		s_RenderData->LightSourceShader->SetFloat3("u_Light.ambient", lightSource.ambient);
		s_RenderData->LightSourceShader->SetFloat3("u_Light.diffuse", lightSource.diffuse);
		s_RenderData->LightSourceShader->SetFloat3("u_Light.specular", lightSource.specular);

		s_RenderData->LightSourceVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->LightSourceVertexArray);
	}
}