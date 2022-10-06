#pragma once

#include <Engine/GPU/VertexArray.h>

namespace gte::GPU::OpenGL {

	/**
	* @brief Class to Represent a VertexArray on OpenGL API
	*/
	class ENGINE_API OpenGLVertexArray : public VertexArray {
	public:

		//Constructor(s) & Destructor
		OpenGLVertexArray() noexcept;
		virtual ~OpenGLVertexArray() noexcept;

		void Bind(void) const noexcept override;
		void Unbind(void) const noexcept override;

		void AddVertexBuffer(const VertexBuffer* vb) noexcept override;

		void SetIndexBuffer(const IndexBuffer* ib) noexcept override;

		[[nodiscard]] const std::vector<VertexBuffer*>& GetVertexBuffers(void) const noexcept override { return mVertexBuffers; }
		[[nodiscard]] const IndexBuffer* GetIndexBuffer(void) const noexcept override { return mIndexBuffer; }

	private:

		/**
		* ID used to distinguish each VertexArrays in OpenGL
		*/
		uint32 mID;

		/**
		* Index of the next VertexBuffer
		*/
		uint32 mVbIndex;

		/**
		* Placeholder for the VertexBuffers
		*/
		std::vector<VertexBuffer*> mVertexBuffers;

		/**
		* Pointer to an IndexBuffer
		*/
		IndexBuffer* mIndexBuffer;

	public:
		//ALLOC
		//DEALLOC
	};

}

