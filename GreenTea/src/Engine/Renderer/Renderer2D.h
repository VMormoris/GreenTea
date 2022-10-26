#pragma once

#include <Engine/Assets/Font.h>
#include <Engine/GPU/Texture.h>

#include <glm.hpp>

namespace gte {
	
	/*
	* @brief Batch Renderer for a 2D workflow
	*/
	class ENGINE_API Renderer2D {
	public:

		/**
		* @brief Initialize the Renderer
		*/
		static void Init(void);

		/**
		* @brief Releasing the resources used by the Renderer
		*/
		static void Shutdown(void);

		/**
		* @brief Starts a new Batch
		* @param eyematrix Eye matrix of the camera used for rendering
		*/
		static void BeginScene(const glm::mat4& eyematrix);

		/**
		* @brief Ends the Batch
		*/
		static void EndScene(void);

		/**
		* @brief Flushes the contents of the Buffer and performs a draw call
		*/
		static void Flush(void);

		/**
		* @brief Load Colored Quad into the Batch
		* @param tranformation Matrix representing Quad's position in 2D world
		* @param color The RGBA values that will be used to fill the Quad
		*/
		static void DrawQuad(const glm::mat4& transformation, uint32 ID, const glm::vec4& color, float tilingFactor = 1.0f);

		/**
		* @brief Load Textured Quad into the Batch
		* @param tranformation Matrix representing Quad's position in 2D world
		* @param texture Texture that will be used for the Quad
		* @param tintColor The RGBA values that will be used to as a tint color for the texture
		*/
		static void DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, uint32 ID, const glm::vec4& tintColor = glm::vec4(1.0f), float tilingFactor = 1.0f);

		/**
		* @brief Load Textured Quad into the Batch
		* @param transformation Matrix reprsenting Quad's position in 2D world
		* @param texture Texture that will be used for the Quad
		* @param textCoords Texture coordinates that will be used for this Quad
		* @param tintColor The RGBA values that will be used to as a tint color for the texture
		*/
		static void DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, const TextureCoordinates& textCoords, uint32 ID, const glm::vec4& tintColor = glm::vec4(1.0f), float tilingFactor = 1.0f);

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, uint32 ID, float thickness = 1.0f, float fade = 0.005f);

		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);

		static void DrawRect(const glm::mat4& transform, const glm::vec4& color);

		static void DrawString(const std::string& text, const glm::mat4 transformation, uint32 size, const GPU::Texture* atlas, const internal::Font* font, uint32 ID, const glm::vec4& color = glm::vec4(1.0f));

		static float GetLineThickness();
		static void SetLineThickness(float width);
	};

}