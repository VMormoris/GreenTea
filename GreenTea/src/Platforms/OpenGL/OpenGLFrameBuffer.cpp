#include "OpenGLFrameBuffer.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <chrono>

static constexpr uint32 MaxSize = 8192;

static int32 FindDepth(const std::vector<gte::GPU::TextureFormat> attachments);

namespace gte::GPU::OpenGL {

	void OpenGLFrameBuffer::Invalidate(void) noexcept
	{
		int32 depthIndex = FindDepth(mSpecification.Attachments);
		let ColorAttachments = static_cast<int32>(depthIndex == -1 ? mSpecification.Attachments.size() : mSpecification.Attachments.size() - 1);
		
		if (mID)
		{
			glDeleteFramebuffers(1, &mID);
			if (ColorAttachments > 0)
			{
				glDeleteTextures(ColorAttachments, mColorAttachmentID);
				delete[] mColorAttachmentID;
			}
			if (depthIndex != -1)
				glDeleteTextures(1, &mDepthAttachmentID);
		}

		glCreateFramebuffers(1, &mID);
		glBindFramebuffer(GL_FRAMEBUFFER, mID);

		ENGINE_ASSERT(mSpecification.Attachments.size() !=0, "Cannot create without any attachements");
		
		int32 MaxColorAttachments;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MaxColorAttachments);
		ENGINE_ASSERT(ColorAttachments <= MaxColorAttachments, "GPU supports up to: ", MaxColorAttachments, " color attachments per buffer!");
		
		if (ColorAttachments > 0)//Has Color attachments
		{
			mColorAttachmentID = new uint32[ColorAttachments];
			glCreateTextures(GL_TEXTURE_2D, ColorAttachments, mColorAttachmentID);
			for (int32 i = 0; i < ColorAttachments; i++)
			{
				if (i == depthIndex) continue;

				let format = mSpecification.Attachments[i];
				let NativeFormat = GetNativeTextureFormat(format);
				glBindTexture(GL_TEXTURE_2D, mColorAttachmentID[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, NativeFormat.first, mSpecification.Width, mSpecification.Height, 0, NativeFormat.second, GetTextureInternalType(format), NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mColorAttachmentID[i], 0);
			}

			GLenum* DrawBuffers = new GLenum[ColorAttachments];
			for (int32 i = 0; i < ColorAttachments; i++)
				DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			glDrawBuffers(ColorAttachments, DrawBuffers);
			delete[] DrawBuffers;
		}

		if (depthIndex != -1)//Has Depth attachment
		{
			let format = mSpecification.Attachments[depthIndex];
			if (format == GPU::TextureFormat::DEPTH24)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &mDepthAttachmentID);
				glBindTexture(GL_TEXTURE_2D, mDepthAttachmentID);
				glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, mSpecification.Width, mSpecification.Height);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthAttachmentID, 0);

			}
			else if (format == GPU::TextureFormat::Shadomap)
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &mDepthAttachmentID);
				glBindTexture(GL_TEXTURE_2D, mDepthAttachmentID);
				glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, mSpecification.Width, mSpecification.Height);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthAttachmentID, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ENGINE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!\n\tError: ", status);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& specification) noexcept
		: mSpecification(specification) { Invalidate(); }

	OpenGLFrameBuffer::~OpenGLFrameBuffer(void) noexcept
	{
		int32 depthIndex = FindDepth(mSpecification.Attachments);
		let ColorAttachments = static_cast<int32>(depthIndex == -1 ? mSpecification.Attachments.size() : mSpecification.Attachments.size() - 1);

		glDeleteFramebuffers(1, &mID);
		if (depthIndex != -1)
			glDeleteTextures(1, &mDepthAttachmentID);
		if (ColorAttachments > 0)
		{
			glDeleteTextures(ColorAttachments, mColorAttachmentID);
			delete[] mColorAttachmentID;
		}
	}

	void OpenGLFrameBuffer::Bind(void) const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, mID); }
	void OpenGLFrameBuffer::BindAttachment(uint32 attachment, uint32 unit) const noexcept
	{
		let format = mSpecification.Attachments[attachment];
		if(format == GPU::TextureFormat::DEPTH24 || format == GPU::TextureFormat::DEPTH24STENCIL8 || format == GPU::TextureFormat::Shadomap)
			glBindTextureUnit(unit, mDepthAttachmentID);
		else
		{
			let depthIndex = FindDepth(mSpecification.Attachments);
			let index = depthIndex != -1 && (int32)attachment >= depthIndex ? attachment - 1 : attachment;
			glBindTextureUnit(unit, mColorAttachmentID[index]);
		}
	}
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
		glClearTexImage(mColorAttachmentID[attachement], 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
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

int32 FindDepth(const std::vector<gte::GPU::TextureFormat> attachments)
{
	for (int32 i = 0; i < attachments.size(); i++)
	{
		let format = attachments[i];
		switch (format)
		{
		using namespace gte::GPU;
		case TextureFormat::DEPTH24:
		case TextureFormat::DEPTH24STENCIL8:
		case TextureFormat::Shadomap:
			return i;
		default:
			break;
		}
	}

	return -1;
}