#include "AssetManager.h"
#include "GeometricMesh.h"

#include <GreenTea/GPU/Mesh.h>

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

	Ref<Asset> AssetManager::RequestMesh(const char* filepath)
	{
		std::string key(filepath);
		Ref<Asset> asset = CreateRef<Asset>(nullptr, AssetType::LOADING);
		Map_Mutex.lock();
		if (GPU_MAP.find(key) != GPU_MAP.end())//Already record on GPU
		{
			if (GPU_MAP[key]->Type == AssetType::MESH)//Already loaded Mesh
				asset = GPU_MAP[key];
		}
		else if (RAM_MAP.find(key) != RAM_MAP.end())
		{
			if (RAM_MAP[key]->Type == AssetType::GEOMETRIC_MESH)
			{
				GeometricMesh* mesh = (GeometricMesh*)RAM_MAP[key]->ActualAsset;
				GPU::Mesh* gpumesh = GPU::Mesh::Create(mesh);
				asset = CreateRef<Asset>(gpumesh, AssetType::MESH);
				GPU_MAP.insert({ key, asset });
			}
		}
		else
		{
			RAM_MAP.insert({ key, CreateRef<Asset>(nullptr, AssetType::LOADING) });
			std::thread handle = std::thread([&](std::string map_key) {
				GeometricMesh* mesh = new GeometricMesh(map_key.c_str());
				Map_Mutex.lock();
				RAM_MAP[map_key] = CreateRef<Asset>(mesh, AssetType::GEOMETRIC_MESH);
				Map_Mutex.unlock();
			}, key);
			handle.detach();
		}
		Map_Mutex.unlock();
		return asset;
	}

	void AssetManager::Init() {}

	void AssetManager::Shutdown() {}

	void AssetManager::Clear(void)
	{
		
	}

	void AssetManager::Clean(void)
	{
	}

	void AssetManager::DeleteTexture(const char* filepath)
	{
		
	}


}