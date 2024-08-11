#pragma once

#include "Core.h"

#include "Achengine/Events/ApplicationEvent.h"
#include "Achengine/ImGui/ImGuiLayer.h"
#include "Achengine/Core/LayerStack.h"
#include "Achengine/Core/Timestep.h"
#include "Events/Event.h"
#include "Window.h"


namespace Achengine
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void Exit();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application* Get() { return s_Instance; }
	private:

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		LayerStack* m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		bool bRunning = true;
		bool bMinimized = false;

		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
}