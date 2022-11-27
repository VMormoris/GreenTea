#include "OpenGLFrameBuffer.h"
#include "OpenGLTexture.h"

#ifndef GT_WEB
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_ES31
	#include <GLFW/glfw3.h>
#endif

#include <chrono>

static constexpr uint32 MaxSize = 8192;

//TODO: Add support for multiple Attachements on the same FrameBuffer

namespace gte::GPU::OpenGL {

	void OpenGLFrameBuffer::Invalidate(void) noexcept
	{
		if (mID)
		{
			glDeleteFramebuffers(1, &mID);
			glDeleteTextures(static_cast<int32>(mSpecification.Attachments.size()), mColorAttachmentID);
			delete[] mColorAttachmentID;
		}

#ifndef GT_WEB
		glCreateFramebuffers(1, &mID);
#else
		glGenFramebuffers(1, &mID);
#endif
		glBindFramebuffer(GL_FRAMEBUFFER, mID);


		ENGINE_ASSERT(mSpecification.Attachments.size()!=0, "Cannot create without any attachements");
		int32 MaxColorAttachments;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxColorAttachments);
		ENGINE_ASSERT(mSpecification.Attachments.size() <= MaxColorAttachments, "GPU supports up to: ", MaxColorAttachments, " color attachments per buffer!");
		
		mColorAttachmentID = new uint32[mSpecification.Attachments.size()];
#ifndef GT_WEB
		glCreateTextures(GL_TEXTURE_2D, static_cast<int32>(mSpecification.Attachments.size()), mColorAttachmentID);
#else
		glGenTextures(static_cast<int32>(mSpecification.Attachments.size()), mColorAttachmentID);
#endif
		for (int32 i = 0; i < mSpecification.Attachments.size(); i++)
		{
			const TextureFormat format = mSpecification.Attachments[i];
			const auto NativeFormat = GetNativeTextureFormat(format);
			glBindTexture(GL_TEXTURE_2D, mColorAttachmentID[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, NativeFormat.first, mSpecification.Width, mSpecification.Height, 0, NativeFormat.second, GetTextureInternalType(format), NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mColorAttachmentID[i], 0);
		}

		GLenum* DrawBuffers = new GLenum[mSpecification.Attachments.size()];
		for (uint32 i = 0; i < mSpecification.Attachments.size(); i++)
			DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		glDrawBuffers(static_cast<int32>(mSpecification.Attachments.size()), DrawBuffers);
		delete[] DrawBuffers;

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ENGINE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!\n\tError: ", status);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& specification) noexcept
		: mSpecification(specification) { Invalidate(); }

	OpenGLFrameBuffer::~OpenGLFrameBuffer(void) noexcept
	{
		glDeleteFramebuffers(1, &mID);
		glDeleteTextures(static_cast<int32>(mSpecification.Attachments.size()), mColorAttachmentID);
		delete[] mColorAttachmentID;
	}

	void OpenGLFrameBuffer::ReadBind(void) const noexcept { glBindFramebuffer(GL_READ_FRAMEBUFFER, mID); }
	void OpenGLFrameBuffer::Bind(void) const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, mID); }
	void OpenGLFrameBuffer::Unbind(void) const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	void OpenGLFrameBuffer::Resize(uint32 width, uint32 height) noexcept
	{
		if (width == 0 || height == 0 || width > MaxSize || height > MaxSize)
		{
			ENGINE_ASSERT(false, "Not valid FrameBuffer size!\n\tWidth: ", width, ", Height: ", height);
			return;
		}

		mSpecification.Width = width;
		mSpecification.Height = height;

		Invalidate();
	}

	void OpenGLFrameBuffer::Clear(uint32 attachement, const void* data) const noexcept
	{
		Bind();
#ifndef GT_WEB
		glClearTexImage(mColorAttachmentID[attachement], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
#else
		const TextureFormat format = mSpecification.Attachments[attachement];
		const auto NativeFormat = GetNativeTextureFormat(format);
		uint32* tdata = new uint32[mSpecification.Width * mSpecification.Height];
		memset(tdata, (uint32)(uint64)data, mSpecification.Width * mSpecification.Height * sizeof(uint32));
		glTexImage2D(GL_TEXTURE_2D, 0, NativeFormat.first, mSpecification.Width, mSpecification.Height, 0, NativeFormat.second, GL_UNSIGNED_INT, tdata);
#endif
	}

	void OpenGLFrameBuffer::GetPixel(uint32 attachment, int32 x, int32 y, void* data) const noexcept
	{
		const TextureFormat format = mSpecification.Attachments[attachment];
		const auto Format = GetNativeTextureFormat(format);
		Bind();
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);
		glReadPixels(x, y, 1, 1, Format.second, GetTextureInternalType(format), data);
	}

	[[nodiscard]] uint64 OpenGLFrameBuffer::GetColorAttachmentID(uint32 attachement) const noexcept { return mColorAttachmentID[attachement]; }
	[[nodiscard]] const FrameBufferSpecification& OpenGLFrameBuffer::GetSpecification(void) const noexcept { return mSpecification; }

}