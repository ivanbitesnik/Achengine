#include "Achenginepch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer.h"

namespace Achengine
{
	Shader* Shader::Create(const std::string& filePath)
	{
		switch (*Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				ACHENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case RendererAPI::API::OpenGL:
			{
				return new OpenGLShader(filePath);
			}
		}

		ACHENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Shader* Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (*Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				ACHENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			}
			case RendererAPI::API::OpenGL:
			{
				return new OpenGLShader(name, vertexSrc, fragmentSrc);
			}
		}

		ACHENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	ShaderLibrary::~ShaderLibrary()
	{
		const bool bEmptied = m_Shaders.empty();
	}

	void ShaderLibrary::Add(Shader* shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	void ShaderLibrary::Add(const std::string& name, Shader* shader)
	{
		ACHENGINE_CORE_ASSERT(Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	Shader* ShaderLibrary::Load(const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;
	}

	Shader* ShaderLibrary::Load(const std::string& name, const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(name, shader);
		return shader;
	}

	Shader* ShaderLibrary::Get(const std::string& name)
	{
		ACHENGINE_CORE_ASSERT(!Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return (m_Shaders.find(name) != m_Shaders.end());
	}
}