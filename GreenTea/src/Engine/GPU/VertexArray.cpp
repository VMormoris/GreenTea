#include "VertexArray.h"
#include "GraphicsContext.h"

#include <Platforms/OpenGL/OpenGLVertexArray.h>

namespace gte::GPU {

	[[nodiscard]] VertexArray* VertexArray::Create(void) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLVertexArray();
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

}