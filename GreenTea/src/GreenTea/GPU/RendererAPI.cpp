#include "RendererAPI.h"
#include "GraphicsContext.h"
#include "GreenTea/Core/Logger.h"

#include "Platforms/OpenGL/OpenGLRendererAPI.h"

namespace GTE::GPU {

	RendererAPI* RendererAPI::Create(void)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLRendererAPI();
		default:
			ENGINE_ASSERT(false, "Only OpenGL is supported currently!");
			return nullptr;
		}
	}

}