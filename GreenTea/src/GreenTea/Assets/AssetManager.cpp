#include "AssetManager.h"

#include <GreenTea/GPU/Texture.h>

#include <string>
#include <unordered_map>
#include <mutex>

namespace GTE {

	static std::unordered_map<std::string, Ref<Asset>> RAM_MAP{};
	static std::unordered_map<std::string, Ref<Asset>> GPU_MAP{};
	static std::mutex Map_Mutex;

	Ref<Asset> AssetManager::RequestTexture(const char* filepath)
	{
		std::string key(filepath);
		Ref<Asset> asset = CreateRef<Asset>(nullptr, AssetType::LOADING);
		Map_Mutex.lock();
		if (GPU_MAP.find(key) != GPU_MAP.end())//Already a Record on GPU's map
		{
			if (GPU_MAP[key]->Type == AssetType::TEXTURE)//Texture already loaded on GPU memory
				asset =  GPU_MAP[key];
		}
		else if (RAM_MAP.find(key) != RAM_MAP.end())//Already a Record on RAM's map
		{
			if (RAM_MAP[key]->Type == AssetType::IMAGE)
			{
				Image* img = (Image*)RAM_MAP[key]->ActualAsset;
				GPU::Texture2D* texture = GPU::Texture2D::Create(*img);
				asset = CreateRef<Asset>(texture, AssetType::TEXTURE);
				GPU_MAP.insert({ key, asset });
			}
		}
		else//Must load Texture from disk
		{
			RAM_MAP.insert({ key, CreateRef<Asset>(nullptr, AssetType::LOADING) });
			std::thread handle = std::thread([&](std::string map_key) {
				Image* img = new Image(map_key.c_str());
				Map_Mutex.lock();
				RAM_MAP[map_key] = CreateRef<Asset>(img, AssetType::IMAGE);
				Map_Mutex.unlock();
			}, key);
			handle.detach();
		}
		Map_Mutex.unlock();
		return asset;
	}

	void AssetManager::Clear(void)
	{
		
	}

	void AssetManager::Clean(void)
	{
		/*std::vector<std::string> CycleBin = {};
		
		//Clean GPU memory from not used assets
		CycleBin.reserve(GPU_MAP.size());
		for(auto [key, asset] : GPU_MAP)//Find which assets to delete
		{ 
			if (asset.Count() == 1)
				CycleBin.push_back(key);
		}

		for (auto& key : CycleBin)
			GPU_MAP.erase(key);
		//The RAM assets with same key need to also be removed
		for (auto& key : CycleBin)
		{
			if (RAM_MAP.find(key) == RAM_MAP.end())
				RAM_MAP.erase(key);
		}

		CycleBin.clear();//Prepare for RAM cleaning

		for (auto [key, ref] : RAM_MAP)//Find RAM assets that need to be deleted
		{
			if (ref.Count() == 1)
			{
				auto& asset = *ref;
				if (asset.Type == AssetType::INVALID || asset.Type == AssetType::MUSIC || asset.Type == AssetType::SOUND)
					CycleBin.push_back(key);
			}
		}

		for (auto& key : CycleBin)
			RAM_MAP.erase(key);*/
	}

	void AssetManager::DeleteTexture(const char* filepath)
	{
		
	}

}