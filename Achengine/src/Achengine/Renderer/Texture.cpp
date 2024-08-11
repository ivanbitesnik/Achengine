#include "Achenginepch.h"
#include "Texture.h"

#include "Achengine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Achengine
{
	Texture2D* Texture2D::Create(uint32_t width, uint32_t height)
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
			return new OpenGLTexture2D(width, height);
		}
		}

		ACHENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture2D* Texture2D::Create(const std::string& path)
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
				return new OpenGLTexture2D(path);
			}
		}

		ACHENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}