#pragma once

#include <Engine/Core/Engine.h>
#include "VertexArray.h"
#include "Framebuffer.h"

#include <glm.hpp>

namespace gte::GPU {

	/**
	* @brief Class for low-level Rendering calls
	*/
	class ENGINE_API RendererAPI {
	public:

		virtual ~RendererAPI(void) = default;

		/**
		* @brief used to initialize the RendererAPI
		*/
		virtual void Init(void) noexcept = 0;

		/**
		* @brief Set the viewport for the Rendering Window
		* @param x Start of Rendering Window on x-axis
		* @param y Start of Rendering Window on y-axis
		* @param width Size of Rendering Window on x-axis
		* @param Height Size of Rendering Window on y-axis
		*/
		virtual void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) noexcept = 0;

		/**
		* @brief Setter for the Color that will be used for clearing Rendering Window
		* @param color RGBA values
		* @warning The RGBA values must be [0, 1]
		*/
		virtual void SetClearColor(const glm::vec4& color) noexcept = 0;

		/**
		* @brief Clears the Rendering Window
		*/
		virtual void Clear(void) noexcept = 0;

		/**
		* @brief Performs a draw call on VertexArray using it's IndexBuffer
		* @param va Pointer to a constant VertexArray
		* @param indices Number of indices that will used for drawning
		*/
		virtual void DrawIndexed(const VertexArray* va, uint32 indices) noexcept = 0;

		virtual void DrawLines(const VertexArray* va, uint32 lines) noexcept = 0;

		virtual void SetLineThickness(float thickness) noexcept = 0;

		virtual void DrawFramebuffer(const FrameBuffer* fbo) noexcept = 0;

		/**
		* @brief Creates a new RendererAPI
		* @return A pointer to new RendererAPI
		*/
		[[nodiscard]] static RendererAPI* Create(void) noexcept;

	};
}
