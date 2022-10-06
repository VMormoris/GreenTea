#include <Engine/Events/Input.h>

#include <Engine/Core/Context.h>

#include <GLFW/glfw3.h>

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

}