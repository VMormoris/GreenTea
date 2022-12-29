#pragma once

#include <Engine/GPU/GraphicsContext.h>

namespace gte {

	struct ENGINE_API WindowSpecification {
		std::string Title;
		int32 X, Y;
		uint32 Width, Height;
		bool Center;
		bool Decorate;
		bool Maximized;
		bool Fullscreen;

		WindowSpecification(const std::string& title = "GreenTea Application", int32 x = 0, int32 y = 0, uint32 width = 1080, uint32 height = 720, bool center = false, bool decorate = true, bool maximized = false, bool fullscreen = false)
			: Title(title), X(x), Y(y), Width(width), Height(height), Center(center), Decorate(decorate), Maximized(maximized), Fullscreen(fullscreen){}
	};

	/**
	* @brief Window interface
	*/
	class ENGINE_API Window {
	public:

		virtual ~Window(void) = default;

		/**
		* @brief Getter for Window's Width
		*/
		[[nodiscard]] virtual uint32 GetWidth(void) const noexcept = 0;

		/**
		* @brief Getter for Window's Height
		*/
		[[nodiscard]] virtual uint32 GetHeight(void) const = 0;

		/**
		* @brief Getter for Window's position on X-axis
		*/
		//virtual int32 GetX(void) const = 0;
		/**
		* @brief Getter for Window's position on Y-axis
		*/
		//virtual int32 GetY(void) const = 0;

		/**
		* @brief Setter for Window's VSync mode
		* @param enableFlag True if VSync must be turned ON, False to be turned OFF
		*/
		virtual void SetVSync(bool enableFlag) noexcept = 0;

		/**
		* @brief Getter for Window's VSync mode
		* @returns A boolean value that That true represents that VSync it is ON and false that it is OFF
		*/
		[[nodiscard]] virtual bool IsVSync(void) const noexcept = 0;

		/**
		* @brieg Getter for pointer to the native Window
		* @returns Pointer to the native Window casted to void*
		*/
		[[nodiscard]] virtual void* GetPlatformWindow(void) noexcept = 0;

		/**
		* @brief Getter for pointer to the native Window
		* @returns Constant pointer to the native Window casted to const void*
		*/
		[[nodiscard]] virtual const void* GetPlatformWindow(void) const noexcept = 0;

		/**
		* @brief Getter for Window's GraphicsContext
		* @return Pointer to the Window's GraphicsContext
		*/
		[[nodiscard]] virtual GPU::GraphicsContext* GetContext(void) noexcept = 0;

		/**
		* @brief Getter for Window's GraphicsContext
		* @return Constant pointer to the Window's GraphicsContext
		*/
		[[nodiscard]] virtual const GPU::GraphicsContext* GetContext(void) const noexcept = 0;

		/**
		* @brief Updates the Window
		*/
		virtual void Update(void) noexcept = 0;

		/**
		* @brief Function for Creating a Window instance
		* @param title Specifies the Window's title
		* @param width Specifies the Window's width
		* @param height Specifies the Window's height
		* @returns A pointer to Window
		*/
		[[nodiscard]] static Window* Create(const WindowSpecification& spec) noexcept;

		//Window handling
		//[[nodiscard]] virtual bool IsMaximized(void) noexcept = 0;
		//virtual void Maximize(void) noexcept = 0;
		//virtual void Minimize(void) noexcept = 0;
		//virtual void Restore(void) noexcept = 0;
		//virtual void Close(void) noexcept = 0;//Produces close event

		[[nodiscard]] virtual void* GetNativeWindow(void) noexcept = 0;
		[[nodiscard]] virtual const void* GetNativeWindow(void) const noexcept = 0;
	};

}