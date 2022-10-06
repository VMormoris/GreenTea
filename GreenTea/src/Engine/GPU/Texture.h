#pragma once

#include <Engine/Assets/Image.h>

#include <glm.hpp>

namespace gte {

	struct ENGINE_API TextureCoordinates {
		glm::vec2 BottomLeft{ 0.0f, 0.0f };
		glm::vec2 BottomRight{ 1.0f, 0.0f };
		glm::vec2 TopRight{ 1.0f, 1.0f };
		glm::vec2 TopLeft{ 0.0f, 1.0f };

		TextureCoordinates(void) = default;

		TextureCoordinates(const glm::vec2& bl, const glm::vec2& br, const glm::vec2& tr, const glm::vec2& tl)
			: BottomLeft(bl), BottomRight(br), TopRight(tr), TopLeft(tl) {}
	};

	namespace GPU {

		enum class TextureFormat : byte {
			INVALID = 0,

			RED8,
			RED16,

			RGB8,
			RGB16,

			RGBA8,
			RGBA16,

			Int8,
			Int16,
			Int32,

			UInt8,
			UInt16,
			UInt32
		};

		/**
		* @brief Class for representing any Texture
		*/
		class ENGINE_API Texture {
		public:

			//Destructor
			virtual ~Texture(void) = default;

			/**
			* @brief Getter for Texture's height
			*/
			[[nodiscard]] virtual uint32 GetHeight(void) const noexcept = 0;

			/**
			* @brief Getter for Texture's width
			*/
			[[nodiscard]] virtual uint32 GetWidth(void) const noexcept = 0;


			/**
			* @brief Getter for Texture's ID
			* @returns An API agnostic ID
			*/
			[[nodiscard]] virtual void* GetID(void) const noexcept = 0;

			/**
			* @brief Loads the GPU with the data of the Texture
			* @param data Pointer to the array containing the data
			* @param size Size of the array containing the data
			*/
			virtual void SetData(void* data, size_t size) noexcept = 0;

			/**
			* @brief Loads the GPU with the data for the Image
			* @param image Image containing the information for the texture
			*/
			virtual void SetData(const Image& image) noexcept = 0;

			/**
			* @brief Binds texture to the specified texture slot
			* @param slot Texture slot for the texture to be bound
			*/
			virtual void Bind(uint32 slot = 0) const noexcept = 0;

		};

		/**
		* @brief Representation of 2D Texture
		*/
		class ENGINE_API Texture2D : public Texture {
		public:

			/**
			* @brief Creates a 2D Texture with default color (White)
			* @param width Texture's width
			* @param height Texture's height
			* @return A pointer to a Texture Object
			*/
			[[nodiscard]] static Texture* Create(uint32 width, unsigned height) noexcept;

			/**
			* Create a 2D Texture from an SDL_Surface
			* @param image Image containing the information for the texture
			* @return A pointer to a Texture Object
			*/
			[[nodiscard]] static Texture2D* Create(const Image& image) noexcept;

			/**
			* @brief Create a 2D Texture while loading it's content from a file
			* @param filepath C-String that specifies the filepath for the Texture
			* @return A pointer to a Texture Object
			*/
			//static Texture* Create(const char* filepath);

		};

	}
}
