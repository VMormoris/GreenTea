#ifndef _OPEN_GL_GRAPHICS_CONTEXT
#define _OPEN_GL_GRAPHICS_CONTEXT

#include "GreenTea/GPU/GraphicsContext.h"
#include <SDL.h>

namespace GTE::GPU::OpenGL {

	/**
	* @brief Class for representing Graphics Context on OpenGL API
	*/
	class ENGINE_API OpenGLGraphicsContext : public GraphicsContext {
	public:

		OpenGLGraphicsContext(void) = default;
		~OpenGLGraphicsContext(void);

		void Init(void* window) override;
		void SwapBuffers(void) override;

		void* Get(void);
		const void* Get(void) const;

	private:

		/**
		* @brief Pointer to the window where Graphics are being drawn
		*/
		SDL_Window* m_Window = nullptr;

		/**
		* @brief OpenGL Context for an SDL Window
		*/
		SDL_GLContext m_Context = nullptr;
	};

}

#endif