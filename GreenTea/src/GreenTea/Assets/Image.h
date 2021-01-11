#ifndef _IMAGE
#define _IMAGE

#include "GreenTea/Core/EngineCore.h"


namespace GTE {

	/**
	* @brief Class for an Image representation
	* @warning Saving an image will always result to a .png format
	*/
	class ENGINE_API Image {
	public:

		//Constructor(s) & Destructor
		Image(void) = default;
		Image(uint32 width, uint32 height, int32 bpp);
		Image(const char* filepath);
		Image(const Image& other);
		Image(Image&& other) noexcept;
		~Image(void);

		/**
		* @brief Size of the image in bytes
		* @returns bytes for the image Data
		*/
		size_t Size(void) const;
		
		/**
		* @brief Getter for Image's width in pixels
		* @returns Number of pixels
		*/
		uint32 GetWidth(void) const;

		/**
		* @brief Getter for Image's height in pixels
		* @returns Number of pixels
		*/
		uint32 GetHeight(void) const;

		/**
		* @brief Getter for Image's pixel buffer
		*/
		void* Data(void);

		/**
		* @brief Getter for Image's pixel buffer
		*/
		const void* Data(void) const;

		/**
		* @brief Getter for a specific pixel of the Image
		* @details Depending on Image the size of a pixel might varied.
		* @tparam T Type of the underlying pixel
		* @param x X-axis coordinate of the pixel
		* @param y Y-axis coordinate of the pixel
		* @returns 
		*/
		template<typename T>
		T& GetPixel(size_t x, size_t y)
		{
			ENGINE_ASSERT(x < _width && y < _height, "Pixel is out of bounds!");
			T* tbuffer = (T*)buffer + y * _width + x;
			return *tbuffer;
		}

		/**
		* @brief Loads an Image from Disc
		* @param filepath File from where the image should be loaded
		*/
		void Load(const char* filepath);
		void Save(const char* filepath);

		int32 GetBytePerPixel(void) const;

		//Assignement operators
		Image& operator=(const Image& rhs);
		Image& operator=(Image&& rhs) noexcept;

	private:

		/**
		* @brief Image's width
		*/
		uint32 m_Width = 0;

		/**
		* @brief Image's height
		*/
		uint32 m_Height = 0;

		/**
		* @brief Bytes for each pixel
		*/
		int32 m_bpp = 0;

		/**
		* @brief Actual Image buffer
		*/
		void* m_Buffer = nullptr;
	};

}
#endif