#include "RendererAPI.h"
#include "GraphicsContext.h"

#include "Platforms/OpenGL/OpenGLRendererAPI.h"

namespace gte::GPU {

	[[nodiscard]] RendererAPI* RendererAPI::Create(void) noexcept
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