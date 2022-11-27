#pragma once

#include <Engine/GPU/Texture.h>

#ifndef GT_WEB
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_ES31
	#include <GLFW/glfw3.h>
#endif

#include <string>

namespace gte::GPU::OpenGL {

	[[nodiscard]] std::pair<GLenum, GLenum> GetNativeTextureFormat(TextureFormat format) noexcept;

	[[nodiscard]] GLenum GetTextureInternalType(TextureFormat format) noexcept;

	[[nodiscard]] size_t GetPixelSize(TextureFormat format) noexcept;

	/**
	* @brief Class for representing a 2D Texture on OpenGL API
	*/
	class OpenGLTexture2D : public Texture2D {
	public:

		//Constructor(s) & Destructor
		OpenGLTexture2D(uint32 width, uint32 height) noexcept;
		OpenGLTexture2D(const Image& image, ImageFormat format) noexcept;
		virtual ~OpenGLTexture2D(void) noexcept;

		[[nodiscard]] uint32 GetHeight(void) const noexcept override { return mHeight; }
		[[nodiscard]] uint32 GetWidth(void) const noexcept override { return mWidth; }
		[[nodiscard]] void* GetID(void) const noexcept override { return (void*)(uint64)mID; }

		void Bind(uint32 slot = 0) const noexcept override;
		void SetData(void* data, size_t size) noexcept override;
		void SetData(const Image& image, ImageFormat format) noexcept override;

	private:

		/**
		* Texture's Width
		*/
		uint32 mWidth;

		/**
		* Texture's Height
		*/
		uint32 mHeight;

		/**
		* internal Format used for the texture
		*/
		GLenum mInternalFormat;

		/**
		* Format that will be used during Rendering the Texture
		*/
		GLenum mDataFormat;

		uint32 mID;

	public:
		//ALLOC
		//DEALLOC
	};

}
