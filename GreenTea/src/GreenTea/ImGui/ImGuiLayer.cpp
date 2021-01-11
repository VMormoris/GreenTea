#include "ImGuiLayer.h"
#include "GreenTea/Core/Logger.h"

#include "Platforms/ImGui/SDLGLImGuiLayer.h"

namespace GTE {

	ImGuiLayer* ImGuiLayer::Create(void)
	{
		switch (GPU::GraphicsContext::GetAPI())
		{
		case GPU::GraphicsAPI::OpenGL: return new SDLGLImGuiLayer();
		default:
			ENGINE_ASSERT(false, "Only OpenGL is supported currently!");
			return nullptr;
		}
	}

}