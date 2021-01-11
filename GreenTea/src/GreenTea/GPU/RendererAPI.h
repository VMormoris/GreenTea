#ifndef _RENDERER_API
#define _RENDERER_API

#include "GreenTea/Core/EngineCore.h"
#include "VertexArray.h"

#include <glm.hpp>

namespace GTE::GPU {

	/**
	* @brief Class for low-level Rendering calls
	*/
	class ENGINE_API RendererAPI {
	public:

		virtual ~RendererAPI(void) = default;

		/**
		* @brief used to initialize the RendererAPI
		*/
		virtual void Init(void) = 0;

		/**
		* @brief Set the viewport for the Rendering Window
		* @param x Start of Rendering Window on x-axis
		* @param y Start of Rendering Window on y-axis
		* @param width Size of Rendering Window on x-axis
		* @param Height Size of Rendering Window on y-axis
		*/
		virtual void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) = 0;

		/**
		* @brief Setter for the Color that will be used for clearing Rendering Window
		* @param color RGBA values
		* @warning The RGBA values must be [0, 1]
		*/
		virtual void SetClearColor(const glm::vec4& color) = 0;

		/**
		* @brief Clears the Rendering Window
		*/
		virtual void Clear(void) = 0;

		/**
		* @brief Performs a draw call on VertexArray using it's IndexBuffer
		* @param va Pointer to a constant VertexArray
		* @param indices Number of indices that will used for drawning
		*/
		virtual void DrawIndexed(const VertexArray* va, uint32 indices) = 0;

		/**
		* @brief Creates a new RendererAPI
		* @return A pointer to new RendererAPI
		*/
		static RendererAPI* Create(void);

	};
}
#endif