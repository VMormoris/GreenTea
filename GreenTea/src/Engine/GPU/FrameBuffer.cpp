#include "FrameBuffer.h"
#include "GraphicsContext.h"

#include <Platforms/OpenGL/OpenGLFrameBuffer.h>

namespace gte::GPU {

	[[nodiscard]] FrameBuffer* FrameBuffer::Create(const FrameBufferSpecification& specification) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLFrameBuffer(specification);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

}