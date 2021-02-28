#ifndef _VERTEX_ARRAY
#define _VERTEX_ARRAY

#include "Buffer.h"

namespace GTE::GPU {

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
		virtual void Bind(void) const = 0;

		/**
		* @brief Unbinds any currently bound Vertex Array
		*/
		virtual void Unbind(void) const = 0;

		/**
		* @brief Add a VertexBuffer
		* @param vb A Vertex Buffer
		*/
		virtual void AddVertexBuffer(const VertexBuffer* vb) = 0;

		/**
		* @brief Setter for the IndexBuffer
		* @param ib An Index Buffer
		*/
		virtual void SetIndexBuffer(const IndexBuffer* ib) = 0;

		/**
		* @brief Getter for Vertex Buffers
		* @returns A vector to VertexBuffer pointers
		*/
		virtual const std::vector<VertexBuffer*>& GetVertexBuffers(void) const = 0;

		/**
		* @brief Getter for the Index Buffer
		* @returns A pointer to IndexBuffer object
		*/
		virtual const IndexBuffer* GetIndexBuffer(void) const = 0;

		/**
		* @brief Creates a new VertexArray
		* @return A pointer to new VertexArray object
		*/
		static VertexArray* Create(void);


	};

}

#endif