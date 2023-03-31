#pragma once
#include "Asset.h"
#include "Image.h"
#include <Engine/Core/Ref.h>

#include <unordered_map>
#include <mutex>

namespace gte {

	class ENGINE_API AssetManager {
	public:
		AssetManager(void) = default;
		~AssetManager(void);

		/**
		* @brief Request an Asset using and id
		* @param id Identifier for the Asset
		*/
		[[nodiscard]] Ref<Asset> RequestAsset(const uuid& id, bool enforceRAM = false);

		/**
		* @brief Deletes every Asset on the AssetManager
		*/
		void Clear(void);

		/**
		* @brief Deletes every Asset that isn't being used
		*/
		void Clean(void);

		/**
		* @brief Delets the specified asset
		*/
		void RemoveAsset(const uuid& id);

#ifndef GT_DIST
		/**
		* @brief Request asset's thumbnail
		* @param id Identifier for the Asset
		*/
		[[nodiscard]] Ref<Asset> RequestThumbnail(const uuid& id);
		void CreateThumbnail(const uuid& id, const Image& img);
#endif
	private:

		[[nodiscard]] Ref<Asset> RequestTexture(const uuid& id);
		[[nodiscard]] Ref<Asset> RequestFont(const uuid& id, bool enforceRAM);
		[[nodiscard]] Ref<Asset> RequestMesh(const uuid& id);
		void LoadFromDisk(const uuid& id, const std::string& filepath);

	private:
#ifndef GT_DIST
		std::unordered_map<uuid, Ref<Asset>> mThumbnails;
#endif
		std::unordered_map<uuid, Ref<Asset>> mRAM;
		std::unordered_map<uuid, Ref<Asset>> mVRAM;
		std::mutex mMapMutex;
	};

}