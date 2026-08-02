#include "Achenginepch.h"
#include "LinuxWindow.h"

#include "Achengine/Events/ApplicationEvent.h"
#include "Achengine/Events/KeyEvent.h"
#include "Achengine/Events/MouseEvent.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <unordered_map>

namespace Achengine
{
	static bool s_GLFWInitialized = false;
	static std::unordered_map<GLFWwindow*, LinuxWindow*> s_WindowInstanceMap;

	static void GLFWErrorCallback(int error, const char* description)
	{
		ACHENGINE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

#ifdef ACHENGINE_PLATFORM_LINUX
	Window* Window::Create(const WindowProps& props)
	{
		return new LinuxWindow(props);
	}
#endif

	LinuxWindow::LinuxWindow(const WindowProps& props)
	{
		Init(props);
	}

	LinuxWindow::~LinuxWindow()
	{
		ShutDown();
	}

	void LinuxWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Height = props.Height;
		m_Data.Width = props.Width;

		ACHENGINE_CORE_INFO("Creating Window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			// TODO glfwTerminate on system shutdown
			int success = glfwInit();
			ACHENGINE_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		s_WindowInstanceMap[m_Window] = this;
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			auto instanceIt = s_WindowInstanceMap.find(window);
			if (instanceIt == s_WindowInstanceMap.end())
			{
				return;
			}

			LinuxWindow* instance = instanceIt->second;
			WindowData& data = instance->m_Data;
			if (!data.EventCallback)
			{
				return;
			}
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			auto instanceIt = s_WindowInstanceMap.find(window);
			if (instanceIt == s_WindowInstanceMap.end())
			{
				return;
			}

			WindowData& data = instanceIt->second->m_Data;
			if (!data.EventCallback)
			{
				return;
			}
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			auto instanceIt = s_WindowInstanceMap.find(window);
			if (instanceIt == s_WindowInstanceMap.end())
			{
				return;
			}

			WindowData& data = instanceIt->second->m_Data;
			if (!data.EventCallback)
			{
				return;
			}

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
				auto instanceIt = s_WindowInstanceMap.find(window);
				if (instanceIt == s_WindowInstanceMap.end())
				{
					return;
				}

				WindowData& data = instanceIt->second->m_Data;
				if (!data.EventCallback)
				{
					return;
				}

				KeyTypedEvent event(keycode);
				data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int modes)
		{
			auto instanceIt = s_WindowInstanceMap.find(window);
			if (instanceIt == s_WindowInstanceMap.end())
			{
				return;
			}

			WindowData& data = instanceIt->second->m_Data;
			if (!data.EventCallback)
			{
				return;
			}

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			auto instanceIt = s_WindowInstanceMap.find(window);
			if (instanceIt == s_WindowInstanceMap.end())
			{
				return;
			}

			WindowData& data = instanceIt->second->m_Data;
			if (!data.EventCallback)
			{
				return;
			}

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			auto instanceIt = s_WindowInstanceMap.find(window);
			if (instanceIt == s_WindowInstanceMap.end())
			{
				return;
			}

			WindowData& data = instanceIt->second->m_Data;
			if (!data.EventCallback)
			{
				return;
			}

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void LinuxWindow::ShutDown()
	{
		s_WindowInstanceMap.erase(m_Window);

		delete m_Context;
		m_Context = nullptr;

		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}

	void LinuxWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void LinuxWindow::SetVSync(bool bEnabled)
	{
		if (bEnabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
	}

	bool LinuxWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

}