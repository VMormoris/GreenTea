#pragma once

#include <Engine/GPU/PixelBuffer.h>

#include <array>

namespace gte::GPU::OpenGL {

	class ENGINE_API OpenGLPixelBuffer : public PixelBuffer {
	public:
		OpenGLPixelBuffer(uint32 width, uint32 height, TextureFormat format);
		~OpenGLPixelBuffer(void) noexcept;

		void Resize(uint32 width, uint32 height) noexcept override;
		void SetFramebuffer(const FrameBuffer* buffer) noexcept override;

		void ReadPixels(uint32 attachment) noexcept override;
		void GetPixel(uint32 attachment, int32 x, int32 y, void* data) const noexcept override;

	private:
		
		std::array<uint32, 3> mIDs;
		const FrameBuffer* mFrameBuffer = nullptr;
		uint32 mWidth, mHeight;
		uint32 mGPUIndex = 0;
		TextureFormat mFormat;
	};

}