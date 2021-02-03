#include "SDLWindow.h"
#include "GreenTea/Core/Logger.h"
#include "GreenTea/GPU/GraphicsContext.h"
#include <SDL_syswm.h>

namespace GTE {
	
	static SDL_Window* WindowPtr = nullptr;

	SDLWindow::SDLWindow(const char* title, int32 width, int32 height)
	{
		SDL_WindowFlags window_flags;
		if (width > 0 || height > 0) window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		else if (width < 0 || height < 0)
		{
			window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
			width = 1280; height = 720;
		}
		else window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);

		ENGINE_ASSERT((SDL_Init(SDL_INIT_EVERYTHING) == 0), "Couldn't Initialize SDL!\n\tError: ", SDL_GetError(), '\n');

		switch (GPU::GraphicsContext::GetAPI())
		{
		case GPU::GraphicsAPI::OpenGL:
		{
			// use Double Buffering
			int status = SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			ENGINE_ASSERT(status != -1, "No double buffering!\n\tError: ", SDL_GetError(), '\n');

			ENGINE_ASSERT((SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != -1), "Couldn't setup profiling!\n\tError: ", SDL_GetError(), '\n');
			// set OpenGL Version (3.3)
			status = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);

			ENGINE_ASSERT(status != -1, "Couldn't set OpenGL major version to 3!\n\tError: ", SDL_GetError(), '\n');
			status = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			ENGINE_ASSERT(status != -1, "Couldn't set OpenGL minor version to 3!\n\tError: ", SDL_GetError(), '\n');
			break;
		}
		default:
			ENGINE_ASSERT(false, "Only OpenGL API is supported currently!");
			break;
		}

		m_Window = SDL_CreateWindow
		(
			title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			(int32) width, (int32) height,
			window_flags
		);

		ENGINE_ASSERT(m_Window != NULL, "Failed to create Window");
		WindowPtr = m_Window;

		if ((width == 0) || (height == 0))
			SDL_GetWindowSize(m_Window, &m_Width, &m_Height);
		else
		{
			m_Width = width;
			m_Height = height;
		}

		//Create Context
		m_GraphicsContext = GPU::GraphicsContext::Create();
		m_GraphicsContext->Init(static_cast<void*>(m_Window));

	}
	
	void SDLWindow::Update(void)
	{
		SDL_GetWindowSize(m_Window, &m_Width, &m_Height);
		m_GraphicsContext->SwapBuffers();
	}

	SDLWindow::~SDLWindow(void)
	{
		delete m_GraphicsContext;
		SDL_DestroyWindow(m_Window);
	}

	void SDLWindow::SetVSync(bool enableFlag)
	{ 
		if (m_VSync != enableFlag)
			SDL_GL_SetSwapInterval(enableFlag ? 1 : 0);
		m_VSync = enableFlag;
	}

	bool SDLWindow::IsVSync(void) const { return m_VSync; }

	uint32 SDLWindow::GetWidth(void) const { return (uint32)m_Width; }
	uint32 SDLWindow::GetHeight(void) const { return (uint32)m_Height; }

	void* SDLWindow::GetPlatformWindow(void) { return static_cast<void*>(m_Window); }
	const void* SDLWindow::GetPlatformWindow(void) const { return GetPlatformWindow(); }

	GPU::GraphicsContext* SDLWindow::GetContext(void) { return m_GraphicsContext; }
	const GPU::GraphicsContext* SDLWindow::GetContext(void) const { return GetContext(); }

	void* Window::GetNativeWindow(void)
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(WindowPtr, &info))
			return info.info.win.window;
		else 
			return NULL;
	}

}