#include "Achenginepch.h"
#include "LinuxInput.h"

#include "Achengine/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Achengine
{
#ifdef ACHENGINE_PLATFORM_LINUX
	Input* Input::s_Instance = new LinuxInput();
#endif

	bool LinuxInput::IsKeyPressedImpl(int keycode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow().GetNativeWindow());
		int state = glfwGetKey(window, keycode);
		return (state == GLFW_PRESS) || (state == GLFW_REPEAT);
	}

	bool LinuxInput::IsMouseButtonPressedImpl(int button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow().GetNativeWindow());
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	float LinuxInput::GetMouseXImpl()
	{
		return GetMousePositionImpl().first;
	}

	float LinuxInput::GetMouseYImpl()
	{
		return GetMousePositionImpl().second;
	}

	std::pair<float, float> LinuxInput::GetMousePositionImpl()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { (float)xPos, (float)yPos };
	}

}