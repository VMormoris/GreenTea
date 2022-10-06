#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace gte {


	Image::Image(uint32 width, uint32 height, int32 bpp) noexcept
		: mWidth(width), mHeight(height), mbpp(bpp)
	{
		mBuffer = ::operator new(Size());
	}

	Image::~Image(void) noexcept
	{
		if (mBuffer) ::operator delete(mBuffer, Size());
		mBuffer = nullptr;
	}

	Image::Image(const Image& other) noexcept
		: mWidth(other.mWidth), mHeight(other.mHeight), mbpp(other.mbpp)
	{
		mBuffer = ::operator new(Size());
		memcpy(mBuffer, other.mBuffer, Size());
	}

	Image::Image(Image&& other) noexcept
		: mWidth(other.mWidth), mHeight(other.mHeight), mbpp(other.mbpp)
	{
		mBuffer = other.mBuffer;

		other.mBuffer = nullptr;
		other.mWidth = 0;
		other.mHeight = 0;
		other.mbpp = 0;
	}

	Image& Image::operator=(const Image& rhs) noexcept
	{
		if (this != &rhs)
		{
			if (mBuffer) ::operator delete(mBuffer, Size());
			mWidth = rhs.mWidth;
			mHeight = rhs.mHeight;
			mbpp = rhs.mbpp;

			mBuffer = ::operator new(Size());
			memcpy(mBuffer, rhs.mBuffer, Size());
		}
		return *this;
	}

	Image& Image::operator=(Image&& rhs) noexcept
	{
		if (this != &rhs)
		{
			if (mBuffer) ::operator delete(mBuffer, Size());
			mWidth = rhs.mWidth;
			mHeight = rhs.mHeight;
			mbpp = rhs.mbpp;

			mBuffer = rhs.mBuffer;
			rhs.mBuffer = nullptr;
			rhs.mWidth = 0;
			rhs.mHeight = 0;
			rhs.mbpp = 0;
		}
		return *this;
	}

	void Image::Load(const char* filepath)
	{
		int32 width, height, channels;
		if (mBuffer) ::operator delete(mBuffer, Size());

		stbi_set_flip_vertically_on_load_thread(1);
		byte* tbuffer = stbi_load(filepath, &width, &height, &channels, 0);
		
		if (!tbuffer)
		{
			GTE_ERROR_LOG("Failed to Load Image: ", filepath);
			mBuffer = nullptr;
			return;
		}

		mWidth = width;
		mHeight = height;
		mbpp = channels;

		mBuffer = ::operator new(Size());
		memcpy(mBuffer, tbuffer, Size());
		stbi_image_free(tbuffer);
	}

	void Image::Load(const byte* buffer, size_t length)
	{
		if (mBuffer) ::operator delete(mBuffer, Size());
		mWidth = *(uint32*)buffer;
		mHeight = *(uint32*)(buffer + 4);
		mbpp = *(int32*)(buffer + 8);
		
		mBuffer = ::operator new(Size());
		memcpy(mBuffer, buffer + 12, Size());
	}

	void Image::Save(const char* filepath)
	{
		if (mBuffer)
			stbi_write_png(filepath, mWidth, mHeight, mbpp, mBuffer, mWidth * mbpp);
	}

	Image::Image(const char* filepath) { Load(filepath); }

	[[nodiscard]] size_t Image::Size(void) const noexcept { return (size_t)mWidth * mHeight * mbpp; }
	[[nodiscard]] uint32 Image::GetWidth(void) const noexcept { return mWidth; }
	[[nodiscard]] uint32 Image::GetHeight(void) const noexcept { return mHeight; }
	[[nodiscard]] int32 Image::GetBytePerPixel(void) const noexcept { return mbpp; }

	[[nodiscard]] void* Image::Data(void) noexcept { return mBuffer; }
	[[nodiscard]] const void* Image::Data(void) const noexcept { return mBuffer; }

}