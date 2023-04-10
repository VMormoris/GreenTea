#include "Texture.h"
#include "GraphicsContext.h"

#include <Platforms/OpenGL/OpenGLTexture.h>


namespace gte::GPU {


	[[nodiscard]] Texture* Texture2D::Create(uint32 width, unsigned height) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLTexture2D(width, height);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	[[nodiscard]] Texture2D* Texture2D::Create(const Image& image, const TextureSpecification& spec) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLTexture2D(image, spec);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	/*Texture* Texture2D::Create(const char* filename)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLTexture2D(filename);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}*/
}