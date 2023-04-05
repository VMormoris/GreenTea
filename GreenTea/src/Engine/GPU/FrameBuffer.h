#pragma once

#include "Texture.h"

#include <vector>

namespace gte::GPU {

	struct ENGINE_API FrameBufferSpecification {
		uint32 Width = 0;
		uint32 Height = 0;
		std::vector<TextureFormat> Attachments;
		bool SwapChainTarget = false;
	};

	class ENGINE_API FrameBuffer {
	public:

		virtual ~FrameBuffer(void) = default;

		virtual void Bind(void) const noexcept = 0;
		virtual void BindAttachment(uint32 attachment, uint32 unit = 0) const noexcept = 0;
		virtual void Unbind(void) const noexcept = 0;

		virtual void Resize(uint32 width, uint32 height) noexcept = 0;

		virtual void SpecifyTarget(uint32 attachment, uint32 target) noexcept = 0;

		[[nodiscard]] virtual const FrameBufferSpecification& GetSpecification(void) const noexcept = 0;

		[[nodiscard]] virtual uint64 GetColorAttachmentID(uint32 attachment) const noexcept = 0;

		virtual void Clear(uint32 attachment, const void* data) const noexcept = 0;
		
		virtual void GetPixel(uint32 attachment, int32 x, int32 y, void* data) const noexcept = 0;

		virtual void ReadPixels(uint32 attachment, void* data) const noexcept = 0;

		[[nodiscard]] static FrameBuffer* Create(const FrameBufferSpecification& specification) noexcept;


	};

}
