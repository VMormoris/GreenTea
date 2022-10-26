#include "PixelBuffer.h"
#include "GraphicsContext.h"

#include <Platforms/OpenGL/OpenGLPixelBuffer.h>

namespace gte::GPU {

	[[nodiscard]] PixelBuffer* PixelBuffer::Create(uint32 width, uint32 height, TextureFormat format) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLPixelBuffer(width, height, format);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

}