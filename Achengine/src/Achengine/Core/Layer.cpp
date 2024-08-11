#include "Achenginepch.h"
#include "Layer.h"

namespace Achengine
{

	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{
	}

	Layer::~Layer()
	{
	}

	void Layer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(Layer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(Layer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(Layer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(Layer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Layer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(Layer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(Layer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Layer::OnWindowResizeEvent));
	}

}