#include "Texture.h"
#include "GraphicsContext.h"
#include "GreenTea/Core/Logger.h"

#include "Platforms/OpenGL/OpenGLTexture.h"


namespace GTE::GPU {


	Texture* Texture2D::Create(uint32 width, unsigned height)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLTexture2D(width, height);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	Texture2D* Texture2D::Create(const Image& image)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLTexture2D(image);
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

	CubicTexture* CubicTexture::Create(const CubeMap& cm)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLCubicTexture(cm);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}
}