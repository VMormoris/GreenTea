#pragma once

#include <GLFW/glfw3.h>

#include <Engine/Core/Window.h>

namespace gte::GLFW {

	class ENGINE_API GLFWWindow : public Window {
	public:
		GLFWWindow(const WindowSpecification& spec) noexcept;
		~GLFWWindow(void) noexcept;

		[[nodiscard]] uint32 GetWidth(void) const noexcept override;
		[[nodiscard]] uint32 GetHeight(void) const noexcept override;

		void SetVSync(bool enableFlag) noexcept override;
		[[nodiscard]] bool IsVSync(void) const noexcept override;

		[[nodiscard]] void* GetPlatformWindow(void) noexcept override;
		[[nodiscard]] const void* GetPlatformWindow(void) const noexcept override;

		[[nodiscard]] GPU::GraphicsContext* GetContext(void) noexcept override;
		[[nodiscard]] const GPU::GraphicsContext* GetContext(void) const noexcept override;

		void Update(void) noexcept override;

		[[nodiscard]] glm::vec2 GetMouseOffset(void) const noexcept;

		[[nodiscard]] void* GetNativeWindow(void) noexcept override;
		[[nodiscard]] const void* GetNativeWindow(void) const noexcept override;

	private:

		void Center(void) noexcept;
		void Fullscreen(void) noexcept;

	private:

		GLFWwindow* mWindow = nullptr;
		GPU::GraphicsContext* mGraphicsContext = nullptr;

		struct WindowProps {
			std::string Title;
			int32 Width = 0;
			int32 Height = 0;
			int32 x = 0;
			int32 y = 0;
			bool VSync = true;

			glm::vec2 MousePos = { 0.0f, 0.0f };//Actually last's frames because event callbacks are called last
		};

		WindowProps mProps;
	};

}