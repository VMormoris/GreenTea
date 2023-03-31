#pragma once

#include <Engine/GPU/VertexArray.h>

#include <glm.hpp>

namespace gte {

	class ENGINE_API RenderCommand {
	public:
		/**
			* @brief Initialization the RendererAPI that will be used int32ernaly for the Rendering calls
			*/
		static void Init(void);

		/**
		* @brief Set the viewport for the Rendering Window
		* @param x Start of Rendering Window on x-axis
		* @param y Start of Rendering Window on y-axis
		* @param width Size of Rendering Window on x-axis
		* @param Height Size of Rendering Window on y-axis
		*/
		static void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height);

		/**
		* @brief Setter for the Color that will be used for clearing Rendering Window
		* @param color RGBA values
		* @warning The RGBA values must be [0, 1]
		*/
		static void SetClearColor(const glm::vec4& color);

		/**
		* @brief Clears the Rendering Window
		*/
		static void Clear(void);

		/**
		* @brief Performs a draw call on VertexArray using it's IndexBuffer
		* @param va Point32er to VertexArray
		* @param indices Number of indices that will used for drawning
		*/
		static void DrawIndexed(const GPU::VertexArray* va, uint32 indices);

		static void DrawArrays(const GPU::VertexArray* va, uint32 vertices);

		static void DrawArrays(const GPU::VertexArray* va, uint32 offset, uint32 count);

		static void DrawLines(const GPU::VertexArray* va, uint32 lines);

		static void SetLineThickness(float thickness);
	};

}