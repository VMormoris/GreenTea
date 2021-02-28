#include "OpenGLGraphicsContext.h"
#include "GreenTea/Core/Logger.h"
#include <GL/glew.h>

namespace GTE::GPU::OpenGL {

	void OpenGLGraphicsContext::Init(void* window)
	{
		m_Window = static_cast<SDL_Window*>(window);
		m_Context = SDL_GL_CreateContext(m_Window);
		ENGINE_ASSERT((SDL_GL_MakeCurrent(m_Window, m_Context) == 0), "Couldn't make Context Current!\n\t\tError code: ", SDL_GetError(), '\n');
		GLenum flag = glewInit();
		ENGINE_ASSERT(flag == GLEW_OK, "Glew failed to initialize: (", flag, ") ", glewGetErrorString(flag));
	}

	OpenGLGraphicsContext::~OpenGLGraphicsContext(void)
	{
		SDL_GL_DeleteContext(m_Context);
	}

	void OpenGLGraphicsContext::SwapBuffers(void)
	{
		SDL_GL_SwapWindow(m_Window);
	}

	void* OpenGLGraphicsContext::Get(void) { return m_Context; }
	const void* OpenGLGraphicsContext::Get(void) const { return Get(); }

}