#include "OpenGLTexture.h"

namespace gte::GPU::OpenGL {

	[[nodiscard]] std::pair<GLenum, GLenum> GetNativeTextureFormat(TextureFormat format) noexcept
	{
		switch (format)
		{
		case TextureFormat::RED8:	return { GL_R8, GL_RED };
		case TextureFormat::RED16:	return { GL_R16, GL_RED };
		case TextureFormat::RGB8:	return { GL_RGB8, GL_RGB };
		case TextureFormat::RGB16:	return { GL_RGB16, GL_RGB };
		case TextureFormat::RGBA8:	return { GL_RGBA8, GL_RGBA };
		case TextureFormat::RGBA16:	return { GL_RGBA16, GL_RGBA };
		case TextureFormat::Int8:	return { GL_R8I, GL_RED_INTEGER };
		case TextureFormat::Int16:	return { GL_R16I, GL_RED_INTEGER };
		case TextureFormat::Int32:	return { GL_R32I, GL_RED_INTEGER };
		case TextureFormat::UInt8:	return { GL_R8UI, GL_RED_INTEGER };
		case TextureFormat::UInt16:	return { GL_R16UI, GL_RED_INTEGER };
		case TextureFormat::UInt32:	return { GL_R32UI, GL_RED_INTEGER };
		default:					return {0, 0};
		}
	}

	[[nodiscard]] GLenum GetTextureInternalType(TextureFormat format) noexcept
	{
		switch (format)
		{
		case TextureFormat::RED8:
		case TextureFormat::RED16:
		case TextureFormat::RGB8:
		case TextureFormat::RGB16:
		case TextureFormat::RGBA8:
		case TextureFormat::RGBA16:
			return GL_UNSIGNED_BYTE;
		case TextureFormat::Int8:
		case TextureFormat::Int16:
		case TextureFormat::Int32:
			return GL_INT;
		case TextureFormat::UInt8:
		case TextureFormat::UInt16:
		case TextureFormat::UInt32:
			return GL_UNSIGNED_INT;
		default:
			return 0;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32 width, uint32 height) noexcept
	{
		mWidth = width; mHeight = height;
		mInternalFormat = GL_RGBA8; mDataFormat = GL_RGBA;

		glGenTextures(1, &mID);
		glTextureStorage2D(mID, 1, mInternalFormat, mWidth, mHeight);

		glTextureParameteri(mID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(mID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(mID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(mID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}

	void OpenGLTexture2D::SetData(const Image& image) noexcept
	{
		uint32 texture_format;
		mInternalFormat = image.GetBytePerPixel();
		switch (image.GetBytePerPixel())
		{
		case 4: // contains alpha channel
			texture_format = GL_RGBA;
			mInternalFormat = GL_RGBA8;
			break;
		case 3: // no alpha channel
			texture_format = GL_RGB;
			mInternalFormat = GL_RGB8;
			break;
		case 1://Grey image
			texture_format = GL_RED;
			mInternalFormat = GL_RED;
		default:
			ENGINE_ASSERT(false, "Not acceptable pixel format: ", (int)mInternalFormat);
			//GTE_ERROR_LOG("Error Bytes per pixel was: ", (int)surface->format->BytesPerPixel, " at ", utils::strip_path(_filename));
		}

		mWidth = image.GetWidth();
		mHeight = image.GetHeight();
		mDataFormat = texture_format;
		unsigned char* data = new unsigned char[image.Size()];
		ENGINE_ASSERT(data != NULL, "Erorr occuried while Allocating memory on the Heap!");

		// flip image
		/*for (uint32 y = 0; y < mHeight; y++)
		{
			size_t row_size = (size_t)mWidth * image.GetBytePerPixel() * sizeof(unsigned char);
			memcpy
			(
				&data[(mHeight - y - 1) * mWidth * image.GetBytePerPixel()],
				&static_cast<const unsigned char*>(image.Data())[y * mWidth * image.GetBytePerPixel()],
				row_size
			);
		}*/
		memcpy(data, image.Data(), image.Size());
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mDataFormat, GL_UNSIGNED_BYTE, data);
		delete[] data;

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//if (mhasMipmaps)
		//{
		//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//	glGenerateMipmap(GL_TEXTURE_2D);
		//}
	}

	OpenGLTexture2D::~OpenGLTexture2D(void) noexcept { glDeleteTextures(1, &mID); }

	void OpenGLTexture2D::Bind(uint32 slot) const noexcept { glBindTextureUnit(slot, mID); }

	void OpenGLTexture2D::SetData(void* data, size_t size) noexcept
	{
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mDataFormat, GL_UNSIGNED_BYTE, data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const Image& image) noexcept { SetData(image); }

}