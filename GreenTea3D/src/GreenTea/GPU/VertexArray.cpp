#include "VertexArray.h"
#include "GraphicsContext.h"
#include "GreenTea/Core/Logger.h"

#include "Platforms/OpenGL/OpenGLVertexArray.h"

namespace GTE::GPU {

	VertexArray* VertexArray::Create(void)
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