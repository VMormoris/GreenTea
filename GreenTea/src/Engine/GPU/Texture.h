#pragma once

#include <Engine/Assets/Image.h>

#include <glm.hpp>

namespace gte {

	struct ENGINE_API TextureCoordinates {
		glm::vec2 BottomLeft{ 0.0f, 0.0f };
		glm::vec2 TopRight{ 1.0f, 1.0f };

		TextureCoordinates(void) = default;

		TextureCoordinates(const glm::vec2& bl, const glm::vec2& tr)
			: BottomLeft(bl), TopRight(tr) {}
	};

	namespace GPU {

		enum class ENGINE_API TextureFormat : byte {
			INVALID = 0,

			RED8,
			RED16,

			RGB8,
			RGB16,

			RG8,
			RG16,

			RGBA8,
			RGBA16,

			RED16F,
			RG16F,
			RGB16F,
			RGBA16F,

			Int8,
			Int16,
			Int32,

			UInt8,
			UInt16,
			UInt32,

			Cube,

			DEPTH24,
			DEPTH24STENCIL8,

			Shadomap,

			Depth = DEPTH24,
			DepthStencil = DEPTH24STENCIL8
		};

		enum class ENGINE_API WrapFilter : byte {
			REPEAT = 0,
			MIRRORED_REPEAT,
			CLAMP_EDGE,
			CLAMP_BORDER,
			MIRROR_CLAMP_EDGE
		};

		enum class ENGINE_API ResizeFilter : byte {
			NEAREAST = 0,
			LINEAR,
			NEAREAST_MIPMAP_NEAREST,
			NEAREST_MIPMAP_LINEAR,
			LINEAR_MIPMAP_LINEAR,
			LINEAR_MIPMAP_NEAREST
		};

		struct ENGINE_API TextureSpecification {
			TextureFormat Format;
			WrapFilter S = WrapFilter::REPEAT, T = WrapFilter::REPEAT, R = WrapFilter::REPEAT;
			ResizeFilter Min = ResizeFilter::NEAREAST, Mag = ResizeFilter::NEAREAST;
			bool PrealocateMipMap = true;

			TextureSpecification(TextureFormat format)
				: Format(format) {}
			TextureSpecification(TextureFormat format, ResizeFilter filter)
				: Format(format), Min(filter), Mag(filter) {}
			TextureSpecification(TextureFormat format, ResizeFilter min, ResizeFilter mag)
				: Format(format), Min(min), Mag(mag) {}
			TextureSpecification(TextureFormat format, WrapFilter wrapmode, ResizeFilter resizemode)
				: Format(format), S(wrapmode), T(wrapmode), R(wrapmode), Min(resizemode), Mag(resizemode) {}
			TextureSpecification(TextureFormat format, WrapFilter wrapmode, ResizeFilter min, ResizeFilter mag)
				: Format(format), S(wrapmode), T(wrapmode), R(wrapmode), Min(min), Mag(mag) {}
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
			virtual void SetData(const Image& image, const TextureSpecification& spec) noexcept = 0;

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
			[[nodiscard]] static Texture2D* Create(const Image& image, const TextureSpecification& spec) noexcept;

			/**
			* @brief Create a 2D Texture while loading it's content from a file
			* @param filepath C-String that specifies the filepath for the Texture
			* @return A pointer to a Texture Object
			*/
			//static Texture* Create(const char* filepath);

		};

	}
}
