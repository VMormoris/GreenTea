#pragma once

#include "Image.h"

#include <array>

namespace GTE {

	/**
	* @brief A container for 6 Images that will be used as a CubeMap
	*/
	class ENGINE_API CubeMap{
	public:
		
		//Constructor(s)
		CubeMap(void) = default;
		CubeMap(const char* filepath);
		

		void Load(const char* filepath);
		//void Save(const char* filepath);

		//Getters
		Image& GetFront();
		const Image& GetFront() const;

		Image& GetBack();
		const Image& GetBack() const;

		Image& GetLeft();
		const Image& GetLeft() const;

		Image& GetRight();
		const Image& GetRight() const;

		Image& GetTop();
		const Image& GetTop() const;

		Image& GetBottom();
		const Image& GetBottom() const;

	private:
		std::array<Image, 6> m_Images;
	};

}
