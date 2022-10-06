#include "AssetManager.h"

#include <Engine/Core/Context.h>
#include <Engine/GPU/Texture.h>

namespace gte {

	[[nodiscard]] Ref<Asset> AssetManager::RequestAsset(const uuid& id)
	{
		if (!internal::GetContext()->AssetWatcher.Exists(id))//No asset with such ID exists
			return CreateRef<Asset>();

		auto filepath = std::filesystem::path(internal::GetContext()->AssetWatcher.GetFilepath(id));

		if (filepath.extension() == ".gtimg")//GPU Assets
			return RequestTexture(id);
		
		Ref<Asset> asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		mMapMutex.lock();
		if (mRAM.find(id) != mRAM.end())//Already record on map (might still loading)
				asset = mRAM.at(id);
		else//Must load from disk
		{
			mRAM.insert({ id, CreateRef<Asset>(nullptr, id, AssetType::LOADING) });
			//Asynchronous loading
			std::thread handle = std::thread([&](uuid key, std::string filepath) {
				Asset raw = internal::Load(filepath);
				mMapMutex.lock();
				mRAM.at(key) = CreateRef<Asset>(raw);
				mMapMutex.unlock();
			}, id, filepath.string());
			handle.detach();
			asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		}
		mMapMutex.unlock();
		return asset;
	}


	[[nodiscard]] Ref<Asset> AssetManager::RequestTexture(const uuid& id)
	{
		Ref<Asset> asset = CreateRef<Asset>(nullptr, id, AssetType::INVALID);
		mMapMutex.lock();
		if (mVRAM.find(id) != mVRAM.end())//Already in GPU
		{
			if (mVRAM.at(id)->Type == AssetType::TEXTURE)
				asset = mVRAM.at(id);
		}
		else if (mRAM.find(id) != mRAM.end())//Cached on RAM
		{
			if (mRAM.at(id)->Type == AssetType::LOADING)//Still loading asset
				asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
			if (mRAM.at(id)->Type == AssetType::IMAGE)//ID doesn't much an Image Asset
			{
				Image* img = (Image*)mRAM.at(id)->Data;
				GPU::Texture2D* texture = GPU::Texture2D::Create(*img);
				asset = CreateRef<Asset>(texture, id, AssetType::TEXTURE, img->Size());
				mVRAM.insert({ id, asset });
			}
		}
		else//Must load from disk first
		{
			mRAM.insert({ id, CreateRef<Asset>(nullptr, id, AssetType::LOADING) });
			std::string filepath = internal::GetContext()->AssetWatcher.GetFilepath(id);
			//Asynchronous loading
			std::thread handle = std::thread([&](uuid key, std::string filepath) {
				Asset raw = internal::Load(filepath);
				mMapMutex.lock();
				mRAM.at(key) = CreateRef<Asset>(raw);
				mMapMutex.unlock();
			}, id, filepath);
			handle.detach();
			asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		}
		mMapMutex.unlock();
		return asset;
	}

	void AssetManager::Clear(void)
	{
		for (auto& [id, asset] : mVRAM)
		{
			delete asset->Data;
			asset->Type = AssetType::INVALID;
			asset->Size = 0;
		}
		mVRAM.clear();
		
		for (auto& [id, asset] : mRAM)
		{
			delete asset->Data;
			asset->Type = AssetType::INVALID;
			asset->Size = 0;
		}
		mRAM.clear();
	}

	void AssetManager::Clean(void)
	{
		
	}

	AssetManager::~AssetManager(void) { Clear(); }

	void AssetManager::RemoveAsset(const uuid& id)
	{
		if (mVRAM.find(id) != mVRAM.end())
		{
			delete mVRAM.at(id)->Data;
			mVRAM.erase(id);
		}

		if (mRAM.find(id) != mRAM.end())
		{
			delete mRAM.at(id)->Data;
			mRAM.erase(id);
		}
	}

}