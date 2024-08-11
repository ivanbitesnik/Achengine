#include "Achenginepch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Achengine
{

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		ACHENGINE_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	OpenGLContext::~OpenGLContext()
	{
		delete m_WindowHandle;
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ACHENGINE_CORE_ASSERT(status, "Failed to initialize Glad!");

		ACHENGINE_CORE_INFO("OpenGL Info:");
		ACHENGINE_CORE_INFO(" Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		ACHENGINE_CORE_INFO(" Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		ACHENGINE_CORE_INFO(" Version: {0}", (const char*)glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}