#include "CubeMap.h"
#include <GreenTea/Core/Logger.h>

namespace GTE {


	void CubeMap::Load(const char* filepath)
	{
		Image atlas(filepath);
		const uint32 tileWidth = atlas.GetWidth() / 4;
		const uint32 tileHeight = atlas.GetHeight() / 3;

		ENGINE_ASSERT(atlas.GetBytePerPixel() == 4, "Only RGBA textures currently supported for skyboxes!");
		//Temporary image
		Image img(tileWidth, tileHeight, 4);
		
		{//Load Left Image
			const uint32 xOffset = 0;
			const uint32 yOffset = tileHeight;
			for (uint32 y = 0; y <tileHeight; y++)
			{
				void* dst = (uint32*)img.Data() + (tileWidth * y);
				void* src = (uint32*)atlas.Data() + (atlas.GetWidth() * (y + yOffset)) + xOffset;
				memcpy(dst, src, sizeof(uint32) * tileWidth);
			}
			m_Images[1] = img;
		}

		{//Load Back Image
			const uint32 xOffset = tileWidth;
			const uint32 yOffset = tileHeight;
			for (uint32 y = 0; y < tileHeight; y++)
			{
				void* dst = (uint32*)img.Data() + (tileWidth * y);
				void* src = (uint32*)atlas.Data() + (atlas.GetWidth() * (y + yOffset)) + xOffset;
				memcpy(dst, src, sizeof(uint32) * tileWidth);
			}
			m_Images[5] = img;
		}

		{//Load Right Image
			const uint32 xOffset = 2 * tileWidth;
			const uint32 yOffset = tileHeight;
			for (uint32 y = 0; y < tileHeight; y++)
			{
				void* dst = (uint32*)img.Data() + (tileWidth * y);
				void* src = (uint32*)atlas.Data() + (atlas.GetWidth() * (y + yOffset)) + xOffset;
				memcpy(dst, src, sizeof(uint32) * tileWidth);
			}
			m_Images[0] = img;
		}

		{//Load Front Image
			const uint32 xOffset = 3 * tileWidth;
			const uint32 yOffset = tileHeight;
			for (uint32 y = 0; y < tileHeight; y++)
			{
				void* dst = (uint32*)img.Data() + (tileWidth * y);
				void* src = (uint32*)atlas.Data() + (atlas.GetWidth() * (y + yOffset)) + xOffset;
				memcpy(dst, src, sizeof(uint32) * tileWidth);
			}
			m_Images[4] = img;
		}

		{//Load Top Image
			const uint32 xOffset = tileWidth;
			const uint32 yOffset = 0;
			for (uint32 y = 0; y < tileHeight; y++)
			{
				void* dst = (uint32*)img.Data() + (tileWidth * y);
				void* src = (uint32*)atlas.Data() + (atlas.GetWidth() * (y + yOffset)) + xOffset;
				memcpy(dst, src, sizeof(uint32) * tileWidth);
			}
			m_Images[2] = img;
		}

		{//Load Bottom Image
			const uint32 xOffset = tileWidth;
			const uint32 yOffset = 2*tileHeight;
			for (uint32 y = 0; y < tileHeight; y++)
			{
				void* dst = (uint32*)img.Data() + (tileWidth * y);
				void* src = (uint32*)atlas.Data() + (atlas.GetWidth() * (y + yOffset)) + xOffset;
				memcpy(dst, src, sizeof(uint32) * tileWidth);
			}
			m_Images[3] = img;
		}
	}

	CubeMap::CubeMap(const char* filepath) { Load(filepath); }

	Image& CubeMap::GetFront(void) { return m_Images[4]; }
	const Image& CubeMap::GetFront(void) const { return m_Images[4]; }

	Image& CubeMap::GetBack(void) { return m_Images[5]; }
	const Image& CubeMap::GetBack(void) const { return m_Images[5]; }

	Image& CubeMap::GetLeft() { return m_Images[1]; }
	const Image& CubeMap::GetLeft() const { return m_Images[1]; }

	Image& CubeMap::GetRight() { return m_Images[0]; }
	const Image& CubeMap::GetRight() const { return m_Images[0]; }

	Image& CubeMap::GetTop() { return m_Images[2]; }
	const Image& CubeMap::GetTop() const { return m_Images[2]; }

	Image& CubeMap::GetBottom() { return m_Images[3]; }
	const Image& CubeMap::GetBottom() const { return m_Images[3]; }

}