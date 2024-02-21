#pragma once

#include <Engine/Core/Engine.h>


namespace gte {

	/// @brief Enumaration for different kind of Images
	enum class ENGINE_API ImageFormat : byte {
		Invalid = 0,
		Sprite,
		Font
	};

	/**
	* @brief Class for an Image representation
	* @warning Saving an image will always result to a .png format
	*/
	class ENGINE_API Image {
	public:

		//Constructor(s) & Destructor
		Image(void) = default;
		Image(uint32 width, uint32 height, int32 bpp) noexcept;
		Image(const char* filepath);
		Image(const Image& other) noexcept;
		Image(Image&& other) noexcept;
		~Image(void) noexcept;

		/**
		* @brief Size of the image in bytes
		* @returns bytes for the image Data
		*/
		[[nodiscard]] size_t Size(void) const noexcept;
		
		/**
		* @brief Getter for Image's width in pixels
		* @returns Number of pixels
		*/
		[[nodiscard]] uint32 GetWidth(void) const noexcept;

		/**
		* @brief Getter for Image's height in pixels
		* @returns Number of pixels
		*/
		[[nodiscard]] uint32 GetHeight(void) const noexcept;

		/**
		* @brief Getter for Image's pixel buffer
		*/
		[[nodiscard]] void* Data(void) noexcept;

		/**
		* @brief Getter for Image's pixel buffer
		*/
		[[nodiscard]] const void* Data(void) const noexcept;

		/**
		* @brief Getter for a specific pixel of the Image
		* @details Depending on Image the size of a pixel might varied.
		* @tparam T Type of the underlying pixel
		* @param x X-axis coordinate of the pixel
		* @param y Y-axis coordinate of the pixel
		* @returns 
		*/
		template<typename T>
		[[nodiscard]] T& GetPixel(size_t x, size_t y)
		{
			ASSERT(x < _width && y < _height, "Pixel is out of bounds!");
			T* tbuffer = (T*)buffer + y * _width + x;
			return *tbuffer;
		}

		/**
		* @brief Loads an Image from Disc
		* @param filepath File from where the image should be loaded
		*/
		void Load(const char* filepath);
		void Load(const byte* buffer);
		void Load(const byte* buffer, int32 length);
		void Load(const byte* buffer, uint32 width, uint32 height, int32 bpp);

		void Save(const char* filepath);

		[[nodiscard]] int32 GetBytePerPixel(void) const noexcept;

		//Assignement operators
		Image& operator=(const Image& rhs) noexcept;
		Image& operator=(Image&& rhs) noexcept;

	private:

		/**
		* @brief Image's width
		*/
		uint32 mWidth = 0;

		/**
		* @brief Image's height
		*/
		uint32 mHeight = 0;

		/**
		* @brief Bytes for each pixel
		*/
		int32 mbpp = 0;

		/**
		* @brief Actual Image buffer
		*/
		byte* mBuffer = nullptr;
	};

}
