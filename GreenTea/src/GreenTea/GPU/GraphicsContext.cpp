#include "GraphicsContext.h"
#include "GreenTea/Core/Logger.h"
#include "Platforms/OpenGL/OpenGLGraphicsContext.h"

namespace GTE::GPU {

	GraphicsAPI GraphicsContext::s_API = GraphicsAPI::OpenGL;


	GraphicsContext* GraphicsContext::Create(void)
	{
		switch (GetAPI())
		{
		case GraphicsAPI::OpenGL: return static_cast<GraphicsContext*>(new OpenGL::OpenGLGraphicsContext());
		default:
			ENGINE_ASSERT(false, "Only OpenGL is supported currently!");
			return nullptr;
		}
	}

	void GraphicsContext::SetAPI(GraphicsAPI API) { s_API = API; }
	const GraphicsAPI GraphicsContext::GetAPI(void) { return s_API; }

}