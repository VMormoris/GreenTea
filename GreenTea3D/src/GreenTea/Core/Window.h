#ifndef _WINDOW
#define _WINDOW

#include "EngineCore.h"
#include "GreenTea/GPU/GraphicsContext.h"

namespace GTE {

	/**
	* @brief Window interface
	*/
	class ENGINE_API Window {
	public:

		virtual ~Window(void) = default;

		/**
		* @brief Getter for Window's Width
		*/
		virtual uint32 GetWidth(void) const = 0;

		/**
		* @brief Getter for Window's Height
		*/
		virtual uint32 GetHeight(void) const = 0;
		
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
		virtual void SetVSync(bool enableFlag) = 0;

		/**
		* @brief Getter for Window's VSync mode
		* @returns A boolean value that That true represents that VSync it is ON and false that it is OFF
		*/
		virtual bool IsVSync(void) const = 0;

		/**
		* @brieg Getter for pointer to the native Window
		* @returns Pointer to the native Window casted to void*
		*/
		virtual void* GetPlatformWindow(void) = 0;

		/**
		* @brief Getter for pointer to the native Window
		* @returns Constant pointer to the native Window casted to const void*
		*/
		virtual const void* GetPlatformWindow(void) const = 0;

		/**
		* @brief Getter for Window's GraphicsContext
		* @return Pointer to the Window's GraphicsContext
		*/
		virtual GPU::GraphicsContext* GetContext(void) = 0;

		/**
		* @brief Getter for Window's GraphicsContext
		* @return Constant pointer to the Window's GraphicsContext
		*/
		virtual const GPU::GraphicsContext* GetContext(void) const = 0;
		
		/**
		* @brief Updates the Window
		*/
		virtual void Update(void) = 0;

		/**
		* @brief Function for Creating a Window instance
		* @param title Specifies the Window's title
		* @param width Specifies the Window's width
		* @param height Specifies the Window's height
		* @returns A pointer to Window
		*/
		static Window* Create(const char* title, int32 width, int32 height);

		static void* GetNativeWindow(void);
	
	};

	

}

#endif