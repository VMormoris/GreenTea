#include "OpenGLTexture.h"
#include <thread>
#include <sstream>

namespace gte::GPU::OpenGL {

	[[nodiscard]] std::pair<GLenum, GLenum> GetNativeTextureFormat(TextureFormat format) noexcept
	{
		switch (format)
		{
		case TextureFormat::RED8:		return { GL_R8, GL_RED };
		case TextureFormat::RED16:		return { GL_R16, GL_RED };
		case TextureFormat::RG8:		return { GL_RG8, GL_RG };
		case TextureFormat::RG16:		return { GL_RG16, GL_RG };
		case TextureFormat::RGB8:		return { GL_RGB8, GL_RGB };
		case TextureFormat::RGB16:		return { GL_RGB16, GL_RGB };
		case TextureFormat::RGBA8:		return { GL_RGBA8, GL_RGBA };
		case TextureFormat::RGBA16:		return { GL_RGBA16, GL_RGBA };
		case TextureFormat::RG16F:		return { GL_RG16F, GL_RG };
		case TextureFormat::RGB16F:		return { GL_RGB16F, GL_RGB };
		case TextureFormat::RGBA16F:	return { GL_RGBA16F, GL_RGBA };
		case TextureFormat::Int8:		return { GL_R8I, GL_RED_INTEGER };
		case TextureFormat::Int16:		return { GL_R16I, GL_RED_INTEGER };
		case TextureFormat::Int32:		return { GL_R32I, GL_RED_INTEGER };
		case TextureFormat::UInt8:		return { GL_R8UI, GL_RED_INTEGER };
		case TextureFormat::UInt16:		return { GL_R16UI, GL_RED_INTEGER };
		case TextureFormat::UInt32:		return { GL_R32UI, GL_RED_INTEGER };
		default:						return {0, 0};
		}
	}

	[[nodiscard]] size_t GetPixelSize(TextureFormat format) noexcept
	{
		switch (format)
		{
		case gte::GPU::TextureFormat::RED8:
		case gte::GPU::TextureFormat::Int8:
		case gte::GPU::TextureFormat::UInt8:
			return 1;
		case gte::GPU::TextureFormat::Int16:
		case gte::GPU::TextureFormat::UInt16:
		case gte::GPU::TextureFormat::RED16:
			return 2;
		case gte::GPU::TextureFormat::Int32:
		case gte::GPU::TextureFormat::UInt32:
			return 4;
		case gte::GPU::TextureFormat::RG8:		return 2;
		case gte::GPU::TextureFormat::RG16:		return 2 * 2;
		case gte::GPU::TextureFormat::RGB8:		return 3;
		case gte::GPU::TextureFormat::RGB16:	return 3 * 2;
		case gte::GPU::TextureFormat::RGBA8:	return 4;
		case gte::GPU::TextureFormat::RGBA16:	return 4 * 2;
		default:								return 0;
		}
	}

	[[nodiscard]] GLfloat GetParameter(WrapFilter wrapmode) noexcept
	{
		switch (wrapmode)
		{
		case WrapFilter::REPEAT:			return GL_REPEAT;
		case WrapFilter::MIRRORED_REPEAT:	return GL_MIRRORED_REPEAT;
		case WrapFilter::CLAMP_EDGE:		return GL_CLAMP_TO_EDGE;
		case WrapFilter::CLAMP_BORDER:		return GL_CLAMP_TO_BORDER;
		case WrapFilter::MIRROR_CLAMP_EDGE:	return GL_MIRROR_CLAMP_TO_EDGE;
		default:							return 0.0f;
		}
	}

	[[nodiscard]] GLfloat GetParameter(ResizeFilter resizemode) noexcept
	{
		switch (resizemode)
		{
		case ResizeFilter::NEAREAST:				return GL_NEAREST;
		case ResizeFilter::LINEAR:					return GL_LINEAR;
		case ResizeFilter::NEAREAST_MIPMAP_NEAREST:	return GL_NEAREST_MIPMAP_NEAREST;
		case ResizeFilter::NEAREST_MIPMAP_LINEAR:	return GL_NEAREST_MIPMAP_LINEAR;
		case ResizeFilter::LINEAR_MIPMAP_LINEAR:	return GL_LINEAR_MIPMAP_LINEAR;
		case ResizeFilter::LINEAR_MIPMAP_NEAREST:	return GL_LINEAR_MIPMAP_NEAREST;
		default:									return 0.0f;
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
		case TextureFormat::RED16F:
		case TextureFormat::RG16F:
		case TextureFormat::RGB16F:
		case TextureFormat::RGBA16F:
			return GL_FLOAT;
		default:
			return 0;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32 width, uint32 height) noexcept
	{
		mWidth = width; mHeight = height;
		mInternalFormat = GL_RGBA8;
		mDataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &mID);
		//glTextureStorage2D(mID, 1, mInternalFormat, mWidth, mHeight);

		glTextureParameteri(mID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(mID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(mID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(mID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}

	void OpenGLTexture2D::SetData(const Image& image, const TextureSpecification& spec) noexcept
	{
		switch (image.GetChannels())
		{
		case 4: // contains alpha channel
			mDataFormat = GL_RGBA;
			mInternalFormat = image.IsHDR() ? GL_RGBA16F : GL_RGBA;
			break;
		case 3: // no alpha channel
			mDataFormat = GL_RGB;
			mInternalFormat = image.IsHDR() ? GL_RGB16F : GL_RGB;
			break;
		case 1:// Grey image
			mDataFormat = GL_RED;
			mInternalFormat = GL_RED;
			break;
		default:
			ENGINE_ASSERT(false, "Not acceptable pixel format: ", (int32)mInternalFormat);
			return;
		}

		mWidth = image.GetWidth();
		mHeight = image.GetHeight();
		
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_2D, mID);
		if (image.GetChannels() == 3 || image.GetChannels() == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		let type = image.IsHDR() ? GL_FLOAT : GL_UNSIGNED_BYTE;
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mDataFormat, type, image.Data());

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetParameter(spec.S));
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetParameter(spec.T));
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetParameter(spec.R));

		ENGINE_ASSERT(spec.Mag == ResizeFilter::LINEAR || spec.Mag == ResizeFilter::NEAREAST, "Not acceptable magnification filter.");
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetParameter(spec.Mag));
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetParameter(spec.Min));

		if (spec.Min == ResizeFilter::NEAREAST_MIPMAP_NEAREST ||
			spec.Min == ResizeFilter::NEAREST_MIPMAP_LINEAR ||
			spec.Min == ResizeFilter::LINEAR_MIPMAP_LINEAR ||
			spec.Min == ResizeFilter::LINEAR_MIPMAP_NEAREST)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	OpenGLTexture2D::~OpenGLTexture2D(void) noexcept { glDeleteTextures(1, &mID); }

	void OpenGLTexture2D::Bind(uint32 slot) const noexcept { glBindTextureUnit(slot, mID); }

	void OpenGLTexture2D::SetData(void* data, size_t size) noexcept
	{
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mDataFormat, GL_UNSIGNED_BYTE, data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const Image& image, const TextureSpecification& spec) noexcept { SetData(image, spec); }

}