#ifndef _OPEN_GL_FRAME_BUFFER
#define _OPEN_GL_FRAME_BUFFER

#include "GreenTea/GPU/FrameBuffer.h"

namespace GTE::GPU::OpenGL {

	class ENGINE_API OpenGLFrameBuffer : public FrameBuffer {
	public:

		OpenGLFrameBuffer(const FrameBufferSpecification& specification);
		~OpenGLFrameBuffer(void);

		void Bind(void) const override;
		void Unbind(void) const override;

		void Resize(uint32 width, uint32 height) override;

		const FrameBufferSpecification& GetSpecification(void) const override;

		uint64 GetColorAttachmentID(uint32 attachement) const override;
		void BindAttachment(uint32 attachment, uint32 slot) const override;

		void Clear(uint32 attachment, const void* data) const override;

		void GetPixel(uint32 attachment, int32 x, int32 y, void* data) const override;

	private:

		void Invalidate(void);

		uint32 m_ID = 0;
		uint32* m_ColorAttachmentID = nullptr;
		uint32 m_DepthAttachmentID = 0;
		FrameBufferSpecification m_Specification;

	};

}

#endif