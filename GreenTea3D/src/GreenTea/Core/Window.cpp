#include "Window.h"
#include "Platforms/SDL/SDLWindow.h"

namespace GTE {

	Window* Window::Create(const char* title, int32 width, int32 height)
	{
		return static_cast<Window*>(new SDLWindow(title, width, height));
	}
}