#pragma once

#include <GreenTea/Core/Ref.h>

namespace GTE {

	/**
	* @brief Enumaration for all kind of Assets
	*/
	enum class ENGINE_API AssetType : byte {
		//Usually this means that the Asset has being destroyed
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
		SOUND = 8
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

	/**
	* @brief AssetManager is the proper way to request Asset Handles using GreenTea engine
	*/
	class ENGINE_API AssetManager {
	public:

		/**
		* @brief Queries for a Asset handle of Texture using the default Texture settings
		* @param filepath A C-style string containg the filepath (the filepath is also used for the key on the underlying map)
		* @returns A Reference to an Asset
		* @warnings To properly used the returned reference you must check the Type
		*/
		static Ref<Asset> RequestTexture(const char* filepath);

		/**
		* @brief Destroyes a Texture from GPU memory as well and it's cached data on RAM
		* @param A C-style string containg the filepath (the filepath is also used for the key on the underlying map)
		*/
		static void DeleteTexture(const char* filepath);

		/**
		* @brief Destroys every Asset that is not used by anyone both from RAM and GPU memory
		*/
		static void Clean(void);

		/**
		* @brief Destroys every Asset from RAM and GPU memory
		*/
		static void Clear(void);

	};


}