#ifndef _RENDERER_2D
#define _RENDERER_2D

#include "GreenTea/GPU/Texture.h"

#include <glm.hpp>

namespace GTE {

	/**
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
		static void DrawQuad(const glm::mat4& transformation, uint32 ID, const glm::vec4& color);

		/**
		* @brief Load Textured Quad into the Batch
		* @param tranformation Matrix representing Quad's position in 2D world
		* @param texture Texture that will be used for the Quad
		* @param tintColor The RGBA values that will be used to as a tint color for the texture
		*/
		static void DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, uint32 ID, const glm::vec4& tintColor = glm::vec4(1.0f));

		/**
		* @brief Load Textured Quad into the Batch
		* @param transformation Matrix reprsenting Quad's position in 2D world
		* @param texture Texture that will be used for the Quad
		* @param textCoords Texture coordinates that will be used for this Quad
		* @param tintColor The RGBA values that will be used to as a tint color for the texture
		*/
		static void DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, const TextureCoordinates& textCoords, uint32 ID, const glm::vec4& tintColor = glm::vec4(1.0f));
	};

}

#endif