#pragma once

#include <Engine/GPU/FrameBuffer.h>

namespace gte::GPU::OpenGL {

	class ENGINE_API OpenGLFrameBuffer : public FrameBuffer {
	public:

		OpenGLFrameBuffer(const FrameBufferSpecification& specification) noexcept;
		~OpenGLFrameBuffer(void) noexcept;

		void Bind(void) const noexcept override;
		void BindAttachment(uint32 attachment, uint32 unit = 0) const noexcept override;
		void Unbind(void) const noexcept override;

		void Resize(uint32 width, uint32 height) noexcept override;

		[[nodiscard]] const FrameBufferSpecification& GetSpecification(void) const noexcept override;

		[[nodiscard]] uint64 GetColorAttachmentID(uint32 attachement) const noexcept override;
		void Clear(uint32 attachment, const void* data) const noexcept override;

		void GetPixel(uint32 attachment, int32 x, int32 y, void* data) const noexcept override;

	private:

		void Invalidate(void) noexcept;

		uint32 mID = 0;
		uint32* mColorAttachmentID = nullptr;
		uint32 mDepthAttachmentID = 0;
		FrameBufferSpecification mSpecification;

	};

}
