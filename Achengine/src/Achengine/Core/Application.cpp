#include "Achenginepch.h"
#include "Application.h"

#include "Achengine/Core/KeyCodes.h"
#include "Achengine/Core/Log.h"
#include "Achengine/Renderer/Renderer.h"
#include "Input.h"

#include <GLFW/glfw3.h>

namespace Achengine
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		ACHENGINE_CORE_ASSERT(!s_Instance, "An application already exists!");
		s_Instance = this;

		m_LayerStack = new LayerStack;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer;
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack->PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack->PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack->PopLayer(layer);
		layer->OnDetach();
	}

	void Application::PopOverlay(Layer* overlay)
	{
		m_LayerStack->PopOverlay(overlay);
		overlay->OnDetach();
	}

	void Application::Run()
	{
		while (bRunning)
		{
			float time = (float)glfwGetTime();	// Platform::GetTime
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!bMinimized)
			{
				for (Layer* layer : *m_LayerStack)
				{
					layer->OnUpdate(timestep);
				}
			}

			// Render thread
			m_ImGuiLayer->Begin();
			for (Layer* layer : *m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}

		Exit();
	}

	void Application::Exit()
	{
		delete m_LayerStack;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Application::OnKeyPressed));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResized));

		for (auto it = m_LayerStack->end(); it != m_LayerStack->begin();)
		{
			(*--it)->OnEvent(e);
			if (e.IsHandled())
			{
				break;
			}
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		bRunning = false;
		return true;
	}

	bool Application::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == ACHENGINE_KEY_ESCAPE)
		{
			bRunning = false;
			return true;
		}

		return false;
	}
	bool Application::OnWindowResized(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			bMinimized = true;
			return false;
		}

		bMinimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}