#include "Achenginepch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Achengine
{
	std::shared_ptr<RendererAPI> RenderCommand::s_RendererAPI = std::make_shared<OpenGLRendererAPI>();
}