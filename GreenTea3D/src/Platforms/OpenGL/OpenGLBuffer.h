#ifndef _OPEN_GL_BUFFER
#define _OPEN_GL_BUFFER

#include "GreenTea/GPU/Buffer.h"

namespace GTE::GPU::OpenGL {

	/**
	* @brief Class for Representing an Vertex Buffer when using OpenGL API
	*/
	class OpenGLVertexBuffer : public VertexBuffer {
	public:

		//Contructor(s) & Destructor
		OpenGLVertexBuffer(const void* vertices, size_t size);
		virtual ~OpenGLVertexBuffer(void);

		void Bind(void) const override;
		void Unbind(void) const override;


		void FillBuffer(const void* vertices, size_t size) override;

		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		const BufferLayout& GetLayout(void) const override { return m_Layout; }

	private:

		uint32 m_ID;

		BufferLayout m_Layout;

	public:
		//ALLOC
		//DEALLOC
	};


	/**
	*@brief Class for Representing an Index Buffer when using OpenGL API
	*/
	class OpenGLIndexBuffer : public IndexBuffer {
	public:

		//Constructor(s) & Destructor
		OpenGLIndexBuffer(const uint32* indices, size_t length);
		virtual ~OpenGLIndexBuffer(void);

		void Bind(void) const override;
		void Unbind(void) const override;

		void FillBuffer(const uint32* indices, size_t length) override;

		size_t GetCount(void) const override { return m_Count; }

	private:

		uint32 m_ID;

		size_t m_Count;

	public:
		//ALLOC
		//DEALLOC
	};
}

#endif