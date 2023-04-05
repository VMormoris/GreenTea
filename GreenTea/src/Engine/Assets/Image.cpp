#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <filesystem>

namespace gte {

	Image::Image(uint32 width, uint32 height, int32 channels, bool hdr) noexcept
		: mWidth(width), mHeight(height), mChannels(channels), mIsHDR(hdr)
	{
		mBuffer = (byte*) ::operator new(Size());
	}

	Image::~Image(void) noexcept
	{
		if (mBuffer) ::operator delete(mBuffer, Size());
		mBuffer = nullptr;
	}

	Image::Image(const Image& other) noexcept
		: mWidth(other.mWidth), mHeight(other.mHeight), mChannels(other.mChannels), mIsHDR(other.mIsHDR)
	{
		mBuffer = (byte*) ::operator new(Size());
		memcpy(mBuffer, other.mBuffer, Size());
	}

	Image::Image(Image&& other) noexcept
		: mWidth(other.mWidth), mHeight(other.mHeight), mChannels(other.mChannels), mIsHDR(other.mIsHDR)
	{
		mBuffer = other.mBuffer;

		other.mBuffer = nullptr;
		other.mWidth = 0;
		other.mHeight = 0;
		other.mChannels = 0;
		other.mIsHDR = false;
	}

	Image& Image::operator=(const Image& rhs) noexcept
	{
		if (this != &rhs)
		{
			if (mBuffer) ::operator delete(mBuffer, Size());
			mWidth = rhs.mWidth;
			mHeight = rhs.mHeight;
			mChannels = rhs.mChannels;
			mIsHDR = rhs.mIsHDR;

			mBuffer = (byte*) ::operator new(Size());
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
			mChannels = rhs.mChannels;
			mIsHDR = rhs.mIsHDR;

			mBuffer = rhs.mBuffer;
			rhs.mBuffer = nullptr;
			rhs.mWidth = 0;
			rhs.mHeight = 0;
			rhs.mChannels = 0;
			rhs.mIsHDR = false;
		}
		return *this;
	}

	void Image::Load(const char* filepath)
	{
		mIsHDR = stbi_is_hdr(filepath);
		int32 width, height, channels;
		if (mBuffer) ::operator delete(mBuffer, Size());

		stbi_set_flip_vertically_on_load_thread(1);
		byte* tbuffer = nullptr;
		if (mIsHDR)
			tbuffer = (byte*)stbi_loadf(filepath, &width, &height, &channels, 0);
		else
			tbuffer = stbi_load(filepath, &width, &height, &channels, 0);
		
		if (!tbuffer)
		{
			GTE_ERROR_LOG("Failed to Load Image: ", filepath);
			mBuffer = nullptr;
			return;
		}

		mWidth = width;
		mHeight = height;
		mChannels = channels;

		mBuffer = (byte*) ::operator new(Size());
		memcpy(mBuffer, tbuffer, Size());
		int result = memcmp(mBuffer, tbuffer, Size());
		stbi_image_free(tbuffer);
	}

	static uint32 sIndex = 0;
	void Image::Load(const byte* buffer)
	{
		if (mBuffer) ::operator delete(mBuffer, Size());
		mWidth = *(uint32*)buffer;
		mHeight = *(uint32*)(buffer + 4);
		mChannels = *(int32*)(buffer + 8);
		mIsHDR = *(bool*)(buffer + 12);

		mBuffer = (byte*) ::operator new(Size());
		memcpy(mBuffer, buffer + 13, Size());
	}

	void Image::Load(const byte* buffer, uint32 width, uint32 height, int32 channels, bool hdr)
	{
		if (mBuffer) ::operator delete(mBuffer, Size());

		mWidth = width;
		mHeight = height;
		mChannels = channels;
		mIsHDR = hdr;
		mBuffer = (byte*) ::operator new(Size());

		memcpy(mBuffer, buffer, Size());
	}

	void Image::Save(const char* filepath)
	{
		if (mBuffer)
			stbi_write_png(filepath, mWidth, mHeight, mChannels, mBuffer, mWidth * GetBytePerPixel());
	}

	Image::Image(const char* filepath) { Load(filepath); }

	[[nodiscard]] size_t Image::Size(void) const noexcept
	{

		return (size_t)mWidth * mHeight * GetBytePerPixel();
	}

	[[nodiscard]] uint32 Image::GetWidth(void) const noexcept { return mWidth; }
	[[nodiscard]] uint32 Image::GetHeight(void) const noexcept { return mHeight; }
	[[nodiscard]] int32 Image::GetChannels(void) const noexcept { return mChannels; }
	[[nodiscard]] int32 Image::GetBytePerPixel(void) const noexcept { return mIsHDR ? mChannels * sizeof(float) : mChannels; }

	[[nodiscard]] void* Image::Data(void) noexcept { return mBuffer; }
	[[nodiscard]] const void* Image::Data(void) const noexcept { return mBuffer; }

}