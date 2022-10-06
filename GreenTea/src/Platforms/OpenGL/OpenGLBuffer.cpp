#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace gte::GPU::OpenGL {

	void OpenGLVertexBuffer::FillBuffer(const void* vertices, size_t size) noexcept
	{
		Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)size, vertices);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* vertices, size_t size) noexcept
	{
		mID = 0;
		glGenBuffers(1, &mID);
		if (vertices != NULL)
		{
			Bind();
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, vertices, GL_STATIC_DRAW);
		}
		else
		{
			Bind();
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, NULL, GL_DYNAMIC_DRAW);
		}
	}

	void OpenGLVertexBuffer::Bind(void) const noexcept { glBindBuffer(GL_ARRAY_BUFFER, mID); }
	void OpenGLVertexBuffer::Unbind(void) const noexcept { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	OpenGLVertexBuffer::~OpenGLVertexBuffer(void) noexcept { glDeleteBuffers(1, &mID); }

	void OpenGLIndexBuffer::FillBuffer(const uint32* indices, size_t length) noexcept
	{
		Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * length, indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32* vertices, size_t size) noexcept
	{
		mID = 0;
		glGenBuffers(1, &mID);
		FillBuffer(vertices, size);
		mCount = size;
	}

	void OpenGLIndexBuffer::Bind(void) const noexcept { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mID); }
	void OpenGLIndexBuffer::Unbind(void) const noexcept { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
	OpenGLIndexBuffer::~OpenGLIndexBuffer(void) noexcept { glDeleteBuffers(1, &mID); }

}
