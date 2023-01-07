#include <Engine/Events/Input.h>

#include <Engine/Core/Context.h>

#include "GLFWWindow.h"

namespace gte {
	
	bool Input::IsMouseButtonPressed(MouseButtonType type)
	{
		Window* window = internal::GetContext()->GlobalWindow;
		int32 status = glfwGetMouseButton((GLFWwindow*)window->GetPlatformWindow(), static_cast<int32>(type));
		return status == GLFW_PRESS;
	}

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		Window* window = internal::GetContext()->GlobalWindow;
		int32 status = glfwGetKey((GLFWwindow*)window->GetPlatformWindow(), static_cast<int32>(keycode));
		return status == GLFW_PRESS || status == GLFW_REPEAT;
	}

	void Input::GetMousePos(uint32& x, uint32& y)
	{
		Window* window = internal::GetContext()->GlobalWindow;
		double rawX, rawY;
		glfwGetCursorPos((GLFWwindow*)window->GetPlatformWindow(), &rawX, &rawY);
		x = static_cast<uint32>(rawX), y = static_cast<uint32>(rawY);
#ifndef GT_DIST
		int32 wx, wy;
		glfwGetWindowPos((GLFWwindow*)window->GetPlatformWindow(), &wx, &wy);
		x += wx, y += wy;
#endif
	}

	void Input::GetMouseOffset(uint32& dx, uint32& dy)
	{
		GLFW::GLFWWindow* window = (GLFW::GLFWWindow*)internal::GetContext()->GlobalWindow;
		glm::vec2 offset = window->GetMouseOffset();
		dx = static_cast<uint32>(offset.x);
		dy = static_cast<uint32>(offset.y);
	}

}