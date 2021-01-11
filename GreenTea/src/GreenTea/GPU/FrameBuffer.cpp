#include "FrameBuffer.h"
#include "GraphicsContext.h"
#include "Platforms/OpenGL/OpenGLFrameBuffer.h"
#include "GreenTea/Core/Logger.h"

namespace GTE::GPU {

	FrameBuffer* FrameBuffer::Create(const FrameBufferSpecification& specification)
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