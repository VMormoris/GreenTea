#pragma once

#include <Engine/Core/Window.h>

#include <GLFW/glfw3.h>

#include <glm.hpp>

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

		void SetTitlebarHovered(bool flag) noexcept override { mProps.TitlebarHovered = flag; }
		[[nodiscard]] bool IsTitlebarHovered(void) const noexcept override { return mProps.TitlebarHovered; }

		[[nodiscard]] bool IsMaximized(void) const noexcept override;

		void Maximize(void) noexcept;

		void Minimize(void) noexcept;
		void Restore(void) noexcept;
		void Close(void) noexcept;

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
			bool TitlebarHovered = false;

			glm::vec2 MousePos = { 0.0f, 0.0f };//Actually last's frames because event callbacks are called last
		};

		WindowProps mProps;
	};

}