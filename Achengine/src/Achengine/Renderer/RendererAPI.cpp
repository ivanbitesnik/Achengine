#include "Achenginepch.h"
#include "RendererAPI.h"

namespace Achengine
{
	std::shared_ptr<RendererAPI::API> RendererAPI::s_API = std::make_shared<RendererAPI::API>(RendererAPI::API::OpenGL);
}