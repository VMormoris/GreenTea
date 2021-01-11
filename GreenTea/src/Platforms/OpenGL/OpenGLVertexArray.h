#ifndef _OPENGL_VERTEX_ARRAY
#define _OPENGL_VERTEX_ARRAY

#include "GreenTea/GPU/VertexArray.h"

namespace GTE::GPU::OpenGL {

	/**
	* @brief Class to Represent a VertexArray on OpenGL API
	*/
	class ENGINE_API OpenGLVertexArray : public VertexArray {
	public:

		//Constructor(s) & Destructor
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		void Bind(void) const override;
		void Unbind(void) const override;

		void AddVertexBuffer(const VertexBuffer* vb) override;

		void SetIndexBuffer(const IndexBuffer* ib) override;

		const std::vector<VertexBuffer*>& GetVertexBuffers(void) const override { return m_VertexBuffers; }
		const IndexBuffer* GetIndexBuffer(void) const override { return m_IndexBuffer; }

	private:

		/**
		* ID used to distinguish each VertexArrays in OpenGL
		*/
		uint32 m_ID;

		/**
		* Index of the next VertexBuffer
		*/
		uint32 m_VbIndex;

		/**
		* Placeholder for the VertexBuffers
		*/
		std::vector<VertexBuffer*> m_VertexBuffers;

		/**
		* Pointer to an IndexBuffer
		*/
		IndexBuffer* m_IndexBuffer;

	public:
		//ALLOC
		//DEALLOC
	};

}

#endif