#include "Achenginepch.h"
#include "WindowsInput.h"

#include "Achengine/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Achengine
{

	Input* Input::s_Instance = new WindowsInput;

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow().GetNativeWindow());
		int state = glfwGetKey(window, keycode);
		return (state == GLFW_PRESS) || (state == GLFW_REPEAT);
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow().GetNativeWindow());
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	float WindowsInput::GetMouseXImpl()
	{
		return GetMousePositionImpl().first;
	}

	float WindowsInput::GetMouseYImpl()
	{
		return GetMousePositionImpl().second;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { (float)xPos, (float)yPos };
	}

}