#pragma once

#include "Buffer.h"

namespace gte::GPU {

	/**
	* @brief Class for Representing a Vertex Array
	*/
	class ENGINE_API VertexArray {
	public:

		//Destructor
		virtual ~VertexArray(void) = default;

		/**
		* @brief Binds the Vertex Array
		*/
		virtual void Bind(void) const noexcept = 0;

		/**
		* @brief Unbinds any currently bound Vertex Array
		*/
		virtual void Unbind(void) const noexcept = 0;

		/**
		* @brief Add a VertexBuffer
		* @param vb A Vertex Buffer
		*/
		virtual void AddVertexBuffer(const VertexBuffer* vb) noexcept = 0;

		/**
		* @brief Setter for the IndexBuffer
		* @param ib An Index Buffer
		*/
		virtual void SetIndexBuffer(const IndexBuffer* ib) noexcept = 0;

		/**
		* @brief Getter for Vertex Buffers
		* @returns A vector to VertexBuffer pointers
		*/
		[[nodiscard]] virtual const std::vector<VertexBuffer*>& GetVertexBuffers(void) const noexcept = 0;

		/**
		* @brief Getter for the Index Buffer
		* @returns A pointer to IndexBuffer object
		*/
		[[nodiscard]] virtual const IndexBuffer* GetIndexBuffer(void) const noexcept = 0;

		/**
		* @brief Creates a new VertexArray
		* @return A pointer to new VertexArray object
		*/
		[[nodiscard]] static VertexArray* Create(void) noexcept;

	};

}
