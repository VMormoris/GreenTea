#ifndef _OPENGL_TEXTURE
#define _OPENGL_TEXTURE

#include "GreenTea/GPU/Texture.h"

#include <GL/glew.h>
#include <string>

namespace GTE::GPU::OpenGL {

	std::pair<GLenum, GLenum> GetNativeTextureFormat(TextureFormat format);

	GLenum GetTextureInternalType(TextureFormat format);

	/**
	* @brief Class for representing a 2D Texture on OpenGL API
	*/
	class OpenGLTexture2D : public Texture2D {
	public:

		//Constructor(s) & Destructor
		OpenGLTexture2D(uint32 width, uint32 height);
		OpenGLTexture2D(const Image& image);
		virtual ~OpenGLTexture2D(void);

		uint32 GetHeight(void) const override { return m_Height; }
		uint32 GetWidth(void) const override { return m_Width; }
		void* GetID(void) const override { return (void*)(uint64)m_ID; }

		void Bind(uint32 slot = 0) const;
		void SetData(void* data, size_t size) override;
		void SetData(const Image& image) override;

	private:

		/**
		* Texture's Width
		*/
		uint32 m_Width;

		/**
		* Texture's Height
		*/
		uint32 m_Height;

		/**
		* internal Format used for the texture
		*/
		GLenum m_InternalFormat;

		/**
		* Format that will be used during Rendering the Texture
		*/
		GLenum m_DataFormat;

		uint32 m_ID;

	public:
		//ALLOC
		//DEALLOC
	};

}

#endif