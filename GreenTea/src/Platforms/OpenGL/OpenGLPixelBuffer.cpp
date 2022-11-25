#include "OpenGLPixelBuffer.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <chrono>
#include <cstring>

namespace gte::GPU::OpenGL {

	OpenGLPixelBuffer::OpenGLPixelBuffer(uint32 width, uint32 height, TextureFormat format)
	{
		mWidth = width;
		mHeight = height;
		mFormat = format;
		size_t size = GetPixelSize(format);
		glGenBuffers(static_cast<uint32>(mIDs.size()), mIDs.data());
	
		for (uint32 id : mIDs)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, id);
			glBufferData(GL_PIXEL_PACK_BUFFER, size * mWidth * mHeight, nullptr, GL_STREAM_READ);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	
	OpenGLPixelBuffer::~OpenGLPixelBuffer(void) noexcept { glDeleteBuffers(static_cast<uint32>(mIDs.size()), mIDs.data()); }

	void OpenGLPixelBuffer::Resize(uint32 width, uint32 height) noexcept
	{
		mWidth = width;
		mHeight = height;
		size_t size = GetPixelSize(mFormat);
		glDeleteBuffers(static_cast<uint32>(mIDs.size()), mIDs.data());

		glGenBuffers(static_cast<uint32>(mIDs.size()), mIDs.data());
		for (uint32 id : mIDs)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, id);
			glBufferData(GL_PIXEL_PACK_BUFFER, size * mWidth * mHeight, nullptr, GL_STREAM_READ);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	void OpenGLPixelBuffer::SetFramebuffer(const FrameBuffer* buffer) noexcept { mFrameBuffer = buffer; }

	void OpenGLPixelBuffer::ReadPixels(uint32 attachment) noexcept
	{
		mGPUIndex = (mGPUIndex + 1) % static_cast<uint32>(mIDs.size());
		const auto Format = GetNativeTextureFormat(mFormat);
		mFrameBuffer->Bind();
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mIDs[mGPUIndex]);
		glReadPixels(0, 0, mWidth, mHeight, Format.second, GetTextureInternalType(mFormat), 0);
		glFlush();
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		mFrameBuffer->Unbind();
	}

	void OpenGLPixelBuffer::GetPixel(uint32 attachment, int32 x, int32 y, void* data) const noexcept
	{
		const uint32 index = (mGPUIndex + mIDs.size() - 1) % static_cast<uint32>(mIDs.size());
		const size_t size = GetPixelSize(mFormat);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mIDs[index]);
		byte* ptr = (byte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		if (ptr)
		{
			if (x >= 0 && y > 0 && x < static_cast<int32>(mWidth) && y < static_cast<int32>(mHeight))
			{
				void* offset = ptr + (mWidth * size) * y + x * size;
				memcpy(data, offset, size);
			}
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

}