#pragma once

#include "FrameBuffer.h"

namespace gte::GPU {

	class ENGINE_API PixelBuffer {
	public:
		virtual ~PixelBuffer(void) = default;

		virtual void Resize(uint32 width, uint32 height) noexcept = 0;
		virtual void SetFramebuffer(const FrameBuffer* buffer) noexcept = 0;

		virtual void ReadPixels(uint32 attachment) noexcept = 0;
		virtual void GetPixel(uint32 attachment, int32 x, int32 y, void* data) const noexcept = 0;

		[[nodiscard]] static PixelBuffer* Create(uint32 width, uint32 height, TextureFormat format) noexcept;

	};

}