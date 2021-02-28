#ifndef _SDL_GL_IMGUI_LAYER
#define _SDL_GL_IMGUI_LAYER

#include "GreenTea/ImGui/ImGuiLayer.h"

#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

namespace GTE {

	class ENGINE_API SDLGLImGuiLayer : public ImGuiLayer {
	public:

		void Init(void* window, GPU::GraphicsContext* context) override;
		void Begin(void) override;
		void End(void) override;
		void Shutdown(void) override;

		ImGuiContext* GetContext(void) override;

	private:
		SDL_Window* m_NativeWindow = nullptr;
	};
}

#endif