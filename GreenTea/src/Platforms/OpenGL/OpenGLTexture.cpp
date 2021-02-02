#include "OpenGLTexture.h"
#include "GreenTea/Core/Logger.h"

namespace GTE::GPU::OpenGL {

	OpenGLTexture2D::OpenGLTexture2D(uint32 width, uint32 height) {
		m_Width = width; m_Height = height;
		m_InternalFormat = GL_RGBA8; m_DataFormat = GL_RGBA;

		glGenTextures(1, &m_ID);
		glTextureStorage2D(m_ID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}

	void OpenGLTexture2D::SetData(const Image& image) {
		uint32 texture_format;
		m_InternalFormat = image.GetBytePerPixel();
		switch (image.GetBytePerPixel())
		{
		case 4: // contains alpha channel
			texture_format = GL_RGBA;
			m_InternalFormat = GL_RGBA8;
			break;
		case 3: // no alpha channel
			texture_format = GL_RGB;
			m_InternalFormat = GL_RGB8;
			break;
		case 1://Grey image
			texture_format = GL_RED;
			m_InternalFormat = GL_RED;
		default:
			ENGINE_ASSERT(false, "Not acceptable pixel format: ", (int)m_InternalFormat);
			//GTE_ERROR_LOG("Error Bytes per pixel was: ", (int)surface->format->BytesPerPixel, " at ", utils::strip_path(_filename));
		}

		m_Width = image.GetWidth();
		m_Height = image.GetHeight();
		m_DataFormat = texture_format;
		unsigned char* data = new unsigned char[image.Size()];
		ENGINE_ASSERT(data != NULL, "Erorr occuried while Allocating memory on the Heap!");

		// flip image
		for (uint32 y = 0; y < m_Height; y++)
		{
			size_t row_size = (size_t)m_Width * image.GetBytePerPixel() * sizeof(unsigned char);
			memcpy
			(
				&data[(m_Height - y - 1) * m_Width * image.GetBytePerPixel()],
				&static_cast<const unsigned char*>(image.Data())[y * m_Width * image.GetBytePerPixel()],
				row_size
			);
		}
		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
		delete[] data;

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//if (m_hasMipmaps)
		//{
		//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//	glGenerateMipmap(GL_TEXTURE_2D);
		//}
	}

	OpenGLTexture2D::~OpenGLTexture2D(void) {
		glDeleteTextures(1, &m_ID);
	}

	void OpenGLTexture2D::Bind(uint32 slot) const {
		glBindTextureUnit(slot, m_ID);
	}

	void OpenGLTexture2D::SetData(void* data, size_t size) {
		glBindTexture(GL_TEXTURE_2D, m_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	OpenGLTexture2D::OpenGLTexture2D(const Image& image) { SetData(image); }

	std::pair<GLenum, GLenum> GetNativeTextureFormat(TextureFormat format)
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
		default:					return { 0, 0 };
		}
	}

	GLenum GetTextureInternalType(TextureFormat format)
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

	int32 FindDepth(const std::vector<TextureFormat> attachments)
	{
		for (int32 i = 0; i<attachments.size(); i++)
		{
			TextureFormat format = attachments[i];
			switch (format)
			{
			case TextureFormat::DEPTH24:
			case TextureFormat::DEPTH24STENCIL8:
			case TextureFormat::Shadowmap:
				return i;
			default:
				break;
			}
		}
		return -1;
	}

}