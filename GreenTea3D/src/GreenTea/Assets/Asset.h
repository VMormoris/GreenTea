#pragma once

#include <GreenTea/Core/EngineCore.h>

namespace GTE {

	/**
	* @brief Enumaration for all kind of Assets
	*/
	enum class ENGINE_API AssetType : byte {
		//Usually this means that the Asset has being destroyed, or not used
		INVALID = 0,
		//When the Asset is still loading either 
		//	from Disk to RAM either from RAM to GPU memory
		LOADING = 1,

		//A Texture Asset on RAM
		IMAGE = 2,
		//A Texture Asset on GPU memory
		TEXTURE = 3,

		//An Asset for tha raw Font file on RAM
		FONT = 4,
		//An Asset for Texture Atlas of the font
		//	with specific size on RAM
		FONT_IMAGE = 5,
		//An Asset for Texture Atlas of the font
		// with specific size on GPU memory
		FONT_TEXTURE = 6,

		//A music file asset
		MUSIC = 7,
		//A sound file asset
		SOUND = 8,

		//A Mesh Asset on RAM
		GEOMETRIC_MESH = 9,

		//A Mesh Asset on GPU Memory
		MESH = 10
	};

	/**
	* @brief A handle to an Asset
	* @details The handle caries with the AssetType so the ActualAsset can
	*	casted to the valid object Type by the user
	*/
	struct ENGINE_API Asset {

		void* ActualAsset = nullptr;
		AssetType Type = AssetType::INVALID;

		Asset(void) = default;

		Asset(void* asset, AssetType type)
			: ActualAsset(asset), Type(type) {}

		Asset(const Asset& other) = default;

	};
}