#pragma once

#include <Engine/GPU/Buffer.h>

namespace gte::GPU::OpenGL {

	/**
	* @brief Class for Representing an Vertex Buffer when using OpenGL API
	*/
	class OpenGLVertexBuffer : public VertexBuffer {
	public:

		//Contructor(s) & Destructor
		OpenGLVertexBuffer(const void* vertices, size_t size) noexcept;
		virtual ~OpenGLVertexBuffer(void) noexcept;

		void Bind(void) const noexcept override;
		void Unbind(void) const noexcept override;

		void FillBuffer(const void* vertices, size_t size) noexcept override;

		void SetLayout(const BufferLayout& layout) noexcept override { mLayout = layout; }
		[[nodiscard]] const BufferLayout& GetLayout(void) const noexcept override { return mLayout; }

	private:

		uint32 mID;

		BufferLayout mLayout;

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
		OpenGLIndexBuffer(const uint32* indices, size_t length) noexcept;
		virtual ~OpenGLIndexBuffer(void) noexcept;

		void Bind(void) const noexcept override;
		void Unbind(void) const noexcept override;

		void FillBuffer(const uint32* indices, size_t length) noexcept override;

		[[nodiscard]] size_t GetCount(void) const noexcept override { return mCount; }

	private:

		uint32 mID;

		size_t mCount;

	public:
		//ALLOC
		//DEALLOC
	};
}
