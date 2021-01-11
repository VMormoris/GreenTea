#ifndef _FRAME_BUFFER
#define _FRAME_BUFFER

#include "Texture.h"
#include <vector>
namespace GTE::GPU {

	struct ENGINE_API FrameBufferSpecification {
		uint32 Width = 0;
		uint32 Height = 0;
		std::vector<TextureFormat> Attachments;
		bool SwapChainTarget = false;
	};

	class ENGINE_API FrameBuffer {
	public:

		virtual ~FrameBuffer(void) = default;

		virtual void Bind(void) const = 0;
		virtual void Unbind(void) const = 0;

		virtual void Resize(uint32 width, uint32 height) = 0;

		virtual const FrameBufferSpecification& GetSpecification(void) const = 0;

		virtual uint64 GetColorAttachmentID(uint32 attachment) const = 0;

		virtual void Clear(uint32 attachment, const void* data) const = 0;

		virtual void GetPixel(uint32 attachment, int32 x, int32 y, void* data) const = 0;

		static FrameBuffer* Create(const FrameBufferSpecification& specification);


	};

}
#endif