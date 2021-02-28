#include "OpenGLBuffer.h"
#include <GL/glew.h>

namespace GTE::GPU::OpenGL {

	void OpenGLVertexBuffer::FillBuffer(const void* vertices, size_t size)
	{
		Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)size, vertices);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* vertices, size_t size) {
		m_ID = 0;
		glGenBuffers(1, &m_ID);
		if (vertices != NULL) {
			Bind();
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, vertices, GL_STATIC_DRAW);
		}
		else {
			Bind();
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, NULL, GL_DYNAMIC_DRAW);
		}
	}

	void OpenGLVertexBuffer::Bind(void) const { glBindBuffer(GL_ARRAY_BUFFER, m_ID); }
	void OpenGLVertexBuffer::Unbind(void) const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	OpenGLVertexBuffer::~OpenGLVertexBuffer(void) { glDeleteBuffers(1, &m_ID); }

	void OpenGLIndexBuffer::FillBuffer(const uint32* indices, size_t length) {
		Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * length, indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32* vertices, size_t size) {
		m_ID = 0;
		glGenBuffers(1, &m_ID);
		FillBuffer(vertices, size);
		m_Count = size;
	}

	void OpenGLIndexBuffer::Bind(void) const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID); }
	void OpenGLIndexBuffer::Unbind(void) const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
	OpenGLIndexBuffer::~OpenGLIndexBuffer(void) { glDeleteBuffers(1, &m_ID); }

}
