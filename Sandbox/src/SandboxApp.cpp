#include <Achengine.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sandbox2D.h"
#include "Sandbox3D.h"

/*class ExampleLayer : public Achengine::Layer
{
public:

	ExampleLayer()
		: Layer("Example")
	{
		m_ShaderLibrary = new Achengine::ShaderLibrary;
		m_CameraController = new Achengine::OrthographicCameraController(1280.0f / 720.0f);
		m_VertexArray = Achengine::VertexArray::Create();

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		Achengine::VertexBuffer* vertexBuffer;
		vertexBuffer = Achengine::VertexBuffer::Create(sizeof(vertices), vertices);
		Achengine::BufferLayout layout = {
			{ Achengine::ShaderDataType::Float3, "a_Position" },
			{ Achengine::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };
		Achengine::IndexBuffer* indexBuffer;
		indexBuffer = Achengine::IndexBuffer::Create(sizeof(indices) / sizeof(uint32_t), indices);
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVertexArray = Achengine::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.75f, -0.75f, 0.0f, 0.0f, 0.0f,
			 0.75f, -0.75f, 0.0f, 1.0f, 0.0f,
			 0.75f,  0.75f, 0.0f, 1.0f, 1.0f,
			-0.75f,  0.75f, 0.0f, 0.0f, 1.0f
		};

		Achengine::VertexBuffer* squareVertexBuffer;
		squareVertexBuffer = Achengine::VertexBuffer::Create(sizeof(squareVertices), squareVertices);
		squareVertexBuffer->SetLayout({
			{ Achengine::ShaderDataType::Float3, "a_Position" },
			{ Achengine::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVertexArray->AddVertexBuffer(squareVertexBuffer);

		unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Achengine::IndexBuffer* squareIndexBuffer;
		squareIndexBuffer = Achengine::IndexBuffer::Create(sizeof(squareIndices) / sizeof(uint32_t), squareIndices);
		m_SquareVertexArray->SetIndexBuffer(squareIndexBuffer);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader = Achengine::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

		std::string flatColorVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0f);
			}
		)";

		m_FlatColorShader = Achengine::Shader::Create("FlatColor", flatColorVertexSrc, flatColorFragmentSrc);

		auto textureShader = m_ShaderLibrary->Load("assets/shaders/Texture.glsl");

		m_Texture = Achengine::Texture2D::Create("assets/textures/Checkerboard.png");

		Achengine::OpenGLShader* OGLTextureShader = (Achengine::OpenGLShader*)textureShader;
		OGLTextureShader->Bind();
		OGLTextureShader->UploadUniformInt("u_Texture", 0);
	}

	~ExampleLayer()
	{
		delete m_ShaderLibrary;
		delete m_Shader;
		delete m_VertexArray;
		delete m_FlatColorShader;
		delete m_SquareVertexArray;
		delete m_Texture;
		delete m_CameraController;
	}

	virtual void OnUpdate(Achengine::Timestep timestep) override
	{
		// Update
		m_CameraController->OnUpdate(timestep);

		// Render
		Achengine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Achengine::RenderCommand::Clear();

		Achengine::Renderer::BeginScene(m_CameraController->GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		Achengine::OpenGLShader* OGLFlatColorShader = (Achengine::OpenGLShader*)m_FlatColorShader;
		OGLFlatColorShader->Bind();
		OGLFlatColorShader->UploadUniformFloat3("u_Color", m_SquareColor);
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.16f, y * 0.16f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;

				Achengine::Renderer::Submit(m_SquareVertexArray, m_FlatColorShader, transform);
			}
		}

		auto textureShader = m_ShaderLibrary->Get("Texture");

		m_Texture->Bind();
		Achengine::Renderer::Submit(m_SquareVertexArray, textureShader, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		if (bRenderTriangle)
		{
			Achengine::Renderer::Submit(m_VertexArray, m_Shader);
		}

		Achengine::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		const bool bPressed = ImGui::Button("Add Triangle", ImVec2(100.0f, 100.0f));
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();

		if (bPressed)
		{
			bRenderTriangle = !bRenderTriangle;
		}
	}

	virtual void OnEvent(Achengine::Event& event) override
	{
		m_CameraController->OnEvent(event);
	}

	virtual bool OnWindowResize(Achengine::WindowResizeEvent& event)
	{
		return false;
	}

private:
	Achengine::ShaderLibrary* m_ShaderLibrary;
	Achengine::Shader* m_Shader;
	Achengine::VertexArray* m_VertexArray;

	Achengine::Shader* m_FlatColorShader;
	Achengine::VertexArray* m_SquareVertexArray;

	Achengine::Texture2D* m_Texture;

	Achengine::OrthographicCameraController* m_CameraController;

	bool bRenderTriangle = false;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};*/

class Sandbox : public Achengine::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer);
		//PushLayer(new Sandbox2D);
		PushLayer(new Sandbox3D);
	}

	~Sandbox()
	{

	}
};

Achengine::Application* Achengine::CreateApplication()
{
	return new Sandbox();
}