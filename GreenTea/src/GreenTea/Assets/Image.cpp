#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "GreenTea/Core/Logger.h"


namespace GTE {


	Image::Image(uint32 width, uint32 height, int32 bpp)
		: m_Width(width), m_Height(height), m_bpp(bpp)
	{
		m_Buffer = ::operator new(Size());
	}

	Image::~Image(void)
	{
		if (m_Buffer) ::operator delete(m_Buffer, Size());
		m_Buffer = nullptr;
	}

	Image::Image(const Image& other) :
		m_Width(other.m_Width), m_Height(other.m_Height), m_bpp(other.m_bpp)
	{
		m_Buffer = ::operator new(Size());
		memcpy(m_Buffer, other.m_Buffer, Size());
	}

	Image::Image(Image&& other) noexcept
		: m_Width(other.m_Width), m_Height(other.m_Height), m_bpp(other.m_bpp)
	{
		m_Buffer = other.m_Buffer;

		other.m_Buffer = nullptr;
		other.m_Width = 0;
		other.m_Height = 0;
		other.m_bpp = 0;
	}

	Image& Image::operator=(const Image& rhs)
	{
		if (this != &rhs)
		{
			if (m_Buffer) ::operator delete(m_Buffer, Size());
			m_Width = rhs.m_Width;
			m_Height = rhs.m_Height;
			m_bpp = rhs.m_bpp;

			m_Buffer = ::operator new(Size());
			memcpy(m_Buffer, rhs.m_Buffer, Size());
		}
		return *this;
	}

	Image& Image::operator=(Image&& rhs) noexcept
	{
		if (this != &rhs)
		{
			if (m_Buffer) ::operator delete(m_Buffer, Size());
			m_Width = rhs.m_Width;
			m_Height = rhs.m_Height;
			m_bpp = rhs.m_bpp;

			m_Buffer = rhs.m_Buffer;
			rhs.m_Buffer = nullptr;
			rhs.m_Width = 0;
			rhs.m_Height = 0;
			rhs.m_bpp = 0;
		}
		return *this;
	}

	void Image::Load(const char* filepath)
	{
		int32 width, height, channels;
		if (m_Buffer) ::operator delete(m_Buffer, Size());

		unsigned char* tbuffer = stbi_load(filepath, &width, &height, &channels, 0);
		
		if (!tbuffer)
		{
			GTE_ERROR_LOG("Failed to Load Image: ", filepath);
			m_Buffer = nullptr;
			return;
		}

		m_Width = width;
		m_Height = height;
		m_bpp = channels;

		m_Buffer = ::operator new(Size());
		memcpy(m_Buffer, tbuffer, Size());
		stbi_image_free(tbuffer);

	}

	void Image::Save(const char* filepath)
	{
		if (m_Buffer)
			stbi_write_png(filepath, m_Width, m_Height, m_bpp, m_Buffer, m_Width * m_bpp);
	}

	Image::Image(const char* filepath) { Load(filepath); }

	size_t Image::Size(void) const { return (size_t)m_Width * m_Height * m_bpp; }
	uint32 Image::GetWidth(void) const { return m_Width; }
	uint32 Image::GetHeight(void) const { return m_Height; }
	int32 Image::GetBytePerPixel(void) const { return m_bpp; }

	void* Image::Data(void) { return m_Buffer; }
	const void* Image::Data(void) const { return m_Buffer; }

}