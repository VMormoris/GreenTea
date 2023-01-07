#include "GLFWWindow.h"

#include <Engine/Core/Context.h>
#include <Engine/Events/Input.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>
#include <stb_image.h>

namespace gte::GLFW {

	static void GLFWErrorCallback(int32 error, const char* description) { GTE_ERROR_LOG("GLFW Error (", error, "): ", description); }

	GLFWWindow::GLFWWindow(const WindowSpecification& spec) noexcept
	{
		mProps.Title = std::string(spec.Title);
		mProps.Width = spec.Width;
		mProps.Height = spec.Height;

		if (internal::GetContext()->WindowCount == 0)
		{
			int32 success = glfwInit();
			ENGINE_ASSERT(success, "Couldn't initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		//Flags setup before window creation
		if (!spec.Decorate)
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		if (spec.Maximized)
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		mWindow = glfwCreateWindow(mProps.Width, mProps.Height, spec.Title.c_str(), nullptr, nullptr);
		internal::GetContext()->WindowCount++;

		if (!(spec.Maximized || spec.Center || spec.Fullscreen))
			glfwSetWindowPos(mWindow, spec.X, spec.Y);

		if (spec.Center)
			Center();
		if (spec.Fullscreen)
			Fullscreen();

		GLFWimage logo[1];
		logo[0].pixels = stbi_load("../Assets/Icons/Logo.png", &logo[0].width, &logo[0].height, 0, 4);
		if (logo[0].pixels != NULL)
		{
			glfwSetWindowIcon(mWindow, 1, logo);
			stbi_image_free(logo[0].pixels);
		}

		mGraphicsContext = GPU::GraphicsContext::Create();
		mGraphicsContext->Init(mWindow);

		glfwGetWindowPos(mWindow, &mProps.x, &mProps.y);
		glfwSetWindowUserPointer(mWindow, &mProps);
		SetVSync(true);
		
		//Window events' Callbacks
		glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window) { internal::GetContext()->Dispatcher.Dispatch<EventType::WindowClose>(); });
		
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
			WindowProps& props = *static_cast<WindowProps*>(glfwGetWindowUserPointer(window));
			props.Width = width;
			props.Height = height;
			internal::GetContext()->Dispatcher.Dispatch<EventType::WindowResize>(width, height);
		});
		
		glfwSetWindowPosCallback(mWindow, [](GLFWwindow* window, int x, int y) {
			WindowProps& props = *static_cast<WindowProps*>(glfwGetWindowUserPointer(window));
			props.x = x;
			props.y = y;
			internal::GetContext()->Dispatcher.Dispatch<EventType::WindowMove>(x, y);
		});
		
		glfwSetWindowFocusCallback(mWindow, [](GLFWwindow* window, int focus) {
			if (focus)
				internal::GetContext()->Dispatcher.Dispatch<EventType::WindowGainFocus>();
			else
				internal::GetContext()->Dispatcher.Dispatch<EventType::WindowLostFocus>();
		});

		//Keyboard events' callback
		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			KeyCode keycode = static_cast<KeyCode>(key);
			switch (action)
			{
			case GLFW_PRESS:
				internal::GetContext()->Dispatcher.Dispatch<EventType::KeyPressed>(keycode);
				break;
			case GLFW_RELEASE:
				internal::GetContext()->Dispatcher.Dispatch<EventType::KeyReleased>(keycode);
				break;
			case GLFW_REPEAT://Ignore for the time being
				break;
			}
		});

		//Mouse events' callbacks
		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
			if (button <= 0 && button >= 4)//Ingored not supported mouse buttons
				return;
			MouseButtonType code = (MouseButtonType)button;
			switch (action)
			{
			case GLFW_PRESS:
			{
				Entity entity = Input::GetHoveredEntity();
				if (entity && entity.HasComponent<NativeScriptComponent>())
					internal::GetContext()->Dispatcher.Dispatch<EventType::Click>(entity);
				internal::GetContext()->Dispatcher.Dispatch<EventType::MouseButtonPressed>(code);
				break;
			}
			case GLFW_RELEASE:
				internal::GetContext()->Dispatcher.Dispatch<EventType::MouseButtonReleased>(code);
				break;
			}
		});

		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xOffset, double yOffset) { internal::GetContext()->Dispatcher.Dispatch<EventType::MouseScroll>((float)xOffset, (float)yOffset); });

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos)
		{
			WindowProps* prop = static_cast<WindowProps*>(glfwGetWindowUserPointer(window));
			static Entity hovered = {};
			static double oldx = 0.0, oldy = 0.0;
			prop->MouseOffset = { xpos - oldx, ypos - oldy };
			oldx = xpos; oldy = ypos;

			Entity entity = Input::GetHoveredEntity();
			if (entity != hovered)
			{
				if (hovered && hovered.HasComponent<NativeScriptComponent>())
					internal::GetContext()->Dispatcher.Dispatch<EventType::HoverExit>(hovered);
				if (entity && entity.HasComponent<NativeScriptComponent>())
					internal::GetContext()->Dispatcher.Dispatch<EventType::HoverEnter>(entity);
			}
			hovered = entity;
			internal::GetContext()->Dispatcher.Dispatch<EventType::MouseMove>((int32)xpos, (int32)ypos);
		});

	}

	void GLFWWindow::Center(void) noexcept
	{
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		if (!primary)
			return;
		int32 x, y, width, height;
		glfwGetMonitorWorkarea(primary, &x, &y, &width, &height);

		x = x + static_cast<int32>((width - mProps.Width) / 2.0f);
		y = y + static_cast<int32>((height - mProps.Height) / 2.0f);
		glfwSetWindowPos(mWindow, x, y);
	}

	void GLFWWindow::Fullscreen(void) noexcept
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (monitor)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(mWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			mProps.Width = mode->width;
			mProps.Height = mode->height;
		}
		else
			GTE_ERROR_LOG("Couldn't get monitor pointer");
	}

	GLFWWindow::~GLFWWindow(void) noexcept
	{
		glfwDestroyWindow(mWindow);
		internal::GetContext()->WindowCount--;

		if (internal::GetContext()->WindowCount == 0)
			glfwTerminate();
	}

	void GLFWWindow::Update(void) noexcept
	{
		glfwPollEvents();
		mGraphicsContext->SwapBuffers();
	}

	void GLFWWindow::SetVSync(bool enableFlag) noexcept
	{
		if (enableFlag)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		mProps.VSync = enableFlag;
	}

	[[nodiscard]] bool GLFWWindow::IsVSync(void) const noexcept { return mProps.VSync; }

	[[nodiscard]] uint32 GLFWWindow::GetWidth(void) const noexcept { return mProps.Width; }
	[[nodiscard]] uint32 GLFWWindow::GetHeight(void) const noexcept { return mProps.Height; }

	[[nodiscard]] void* GLFWWindow::GetPlatformWindow(void) noexcept { return static_cast<void*>(mWindow); }
	[[nodiscard]] const void* GLFWWindow::GetPlatformWindow(void) const noexcept { return GetPlatformWindow(); }

	[[nodiscard]] GPU::GraphicsContext* GLFWWindow::GetContext(void) noexcept { return mGraphicsContext; }
	[[nodiscard]] const GPU::GraphicsContext* GLFWWindow::GetContext(void) const noexcept { return mGraphicsContext; }

	[[nodiscard]] void* GLFWWindow::GetNativeWindow(void) noexcept { return static_cast<void*>(glfwGetWin32Window(mWindow)); }

	[[nodiscard]] const void* GLFWWindow::GetNativeWindow(void) const noexcept { return GetNativeWindow(); }

}

namespace gte {
	[[nodiscard]] Window* Window::Create(const WindowSpecification& spec) noexcept { return static_cast<Window*>(new GLFW::GLFWWindow(spec)); }
}