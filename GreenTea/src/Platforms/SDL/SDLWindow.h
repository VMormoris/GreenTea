#ifndef _SDL_WINDOW
#define _SDL_WINDOW

#include "GreenTea/Core/Window.h"
#include <SDL.h>

namespace GTE {

	class ENGINE_API SDLWindow : public Window{
	public:

		SDLWindow(const char* title, int32 width, int32 height);
		~SDLWindow(void);

		uint32 GetWidth(void) const override;
		uint32 GetHeight(void) const override;

		void SetVSync(bool enableFlag) override;
		bool IsVSync(void) const override;

		void* GetPlatformWindow(void) override;
		const void* GetPlatformWindow(void) const override;

		GPU::GraphicsContext* GetContext(void) override;
		const GPU::GraphicsContext* GetContext(void) const override;

		void Update(void) override;

	private:
		
		/**
		* @brief Window's Width
		*/
		int32 m_Width;

		/**
		* @brief Window's Height
		*/
		int32 m_Height;

		/**
		* @brief VSync flag
		*/
		bool m_VSync = true;

		/**
		* @brief Pointer to an SDL Window (Native Window)
		*/
		SDL_Window* m_Window = NULL;

		//TODO: Change to Graphic Context when Implemented
		 GPU::GraphicsContext* m_GraphicsContext;
	};

}

#endif