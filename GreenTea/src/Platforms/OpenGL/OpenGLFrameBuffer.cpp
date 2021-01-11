#include "OpenGLFrameBuffer.h"
#include "GreenTea/Core/Logger.h"
#include "OpenGLTexture.h"

#include <GL/glew.h>

static const uint32 MaxSize = 8192;

//TODO: Add support for multiple Attachements on the same FrameBuffer

namespace GTE::GPU::OpenGL {

	void OpenGLFrameBuffer::Invalidate(void)
	{
		if (m_ID)
		{
			glDeleteFramebuffers(1, &m_ID);
			glDeleteTextures(static_cast<int32>(m_Specification.Attachments.size()), m_ColorAttachmentID);
			delete[] m_ColorAttachmentID;
		}

		glCreateFramebuffers(1, &m_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);


		ENGINE_ASSERT(m_Specification.Attachments.size()!=0, "Cannot create without any attachements");
		int32 MaxColorAttachments;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxColorAttachments);
		ENGINE_ASSERT(m_Specification.Attachments.size() <= MaxColorAttachments, "GPU supports up to: ", MaxColorAttachments, " color attachments per buffer!");
		
		m_ColorAttachmentID = new uint32[m_Specification.Attachments.size()];
		glCreateTextures(GL_TEXTURE_2D, static_cast<int32>(m_Specification.Attachments.size()), m_ColorAttachmentID);
		for (int32 i = 0; i < m_Specification.Attachments.size(); i++)
		{
			const TextureFormat format = m_Specification.Attachments[i];
			const auto NativeFormat = GetNativeTextureFormat(format);
			glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentID[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, NativeFormat.first, m_Specification.Width, m_Specification.Height, 0, NativeFormat.second, GetTextureInternalType(format), NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorAttachmentID[i], 0);
		}

		GLenum* DrawBuffers = new GLenum[m_Specification.Attachments.size()];
		for (uint32 i = 0; i < m_Specification.Attachments.size(); i++)
			DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		glDrawBuffers(static_cast<int32>(m_Specification.Attachments.size()), DrawBuffers);
		delete[] DrawBuffers;

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ENGINE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!\n\tError: ", status);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& specification)
		: m_Specification(specification) { Invalidate(); }

	OpenGLFrameBuffer::~OpenGLFrameBuffer(void)
	{
		glDeleteFramebuffers(1, &m_ID);
		glDeleteTextures(static_cast<int32>(m_Specification.Attachments.size()), m_ColorAttachmentID);
		delete[] m_ColorAttachmentID;
	}

	void OpenGLFrameBuffer::Bind(void) const { glBindFramebuffer(GL_FRAMEBUFFER, m_ID); }
	void OpenGLFrameBuffer::Unbind(void) const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	void OpenGLFrameBuffer::Resize(uint32 width, uint32 height)
	{
		if (width == 0 || height == 0 || width > MaxSize || height > MaxSize)
		{
			ENGINE_ASSERT(false, "Not valid FrameBuffer size!\n\tWidth: ", width, ", Height: ", height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	void OpenGLFrameBuffer::Clear(uint32 attachement, const void* data) const
	{
		Bind();
		glClearTexImage(m_ColorAttachmentID[attachement], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
	}

	void OpenGLFrameBuffer::GetPixel(uint32 attachment, int32 x, int32 y, void* data) const
	{
		const TextureFormat format = m_Specification.Attachments[attachment];
		const auto Format = GetNativeTextureFormat(format);
		Bind();
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glReadPixels(x, y, 1, 1, Format.second, GetTextureInternalType(format), data);
	}

	uint64 OpenGLFrameBuffer::GetColorAttachmentID(uint32 attachement) const { return m_ColorAttachmentID[attachement]; }
	const FrameBufferSpecification& OpenGLFrameBuffer::GetSpecification(void) const { return m_Specification; }

}