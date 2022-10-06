#include "GraphicsContext.h"

#include <Engine/Core/Context.h>

#include <Platforms/OpenGL/OpenGLGraphicsContext.h>

namespace gte::GPU {

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

	void GraphicsContext::SetAPI(GraphicsAPI API) noexcept { internal::GetContext()->G_API = API; }
	const GraphicsAPI GraphicsContext::GetAPI(void) noexcept { return internal::GetContext()->G_API; }

}