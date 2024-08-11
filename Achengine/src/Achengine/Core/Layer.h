#pragma once

#include "Achengine/Core/Core.h"
#include "Achengine/Core/Timestep.h"
#include "Achengine/Events/Event.h"
#include "Achengine/Events/ApplicationEvent.h"
#include "Achengine/Events/KeyEvent.h"
#include "Achengine/Events/MouseEvent.h"

namespace Achengine
{

	class ACHENGINE_API Layer
	{
	public:

		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event);

		virtual bool OnMouseMovedEvent(MouseMovedEvent& event) { return false; }
		virtual bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event) { return false; }
		virtual bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event) { return false; }
		virtual bool OnMouseScrolledEvent(MouseScrolledEvent& event) { return false; }
		virtual bool OnKeyPressedEvent(KeyPressedEvent& event) { return false; }
		virtual bool OnKeyReleasedEvent(KeyReleasedEvent& event) { return false; }
		virtual bool OnKeyTypedEvent(KeyTypedEvent& event) { return false; }
		virtual bool OnWindowResizeEvent(WindowResizeEvent& event) { return false; }

		inline const std::string& GetName() const { return m_DebugName; }
	protected:

		std::string m_DebugName;
	};

}

