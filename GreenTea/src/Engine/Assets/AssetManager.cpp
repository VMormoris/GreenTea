#include "AssetManager.h"
#include "Font.h"

#include <Engine/Core/Context.h>
#include <Engine/GPU/Texture.h>
#include <Engine/GPU/Mesh.h>

namespace gte {

	[[nodiscard]] Ref<Asset> AssetManager::RequestAsset(const uuid& id, bool enforceRAM)
	{
		if (!internal::GetContext()->AssetWatcher.Exists(id))//No asset with such ID exists
			return CreateRef<Asset>(nullptr, id, AssetType::INVALID);

		let filepath = std::filesystem::path(internal::GetContext()->AssetWatcher.GetFilepath(id));

		if (filepath.extension() == ".gtimg")//GPU Assets
			return RequestTexture(id);
		
		if (filepath.extension() == ".gtfont")
			return RequestFont(id, enforceRAM);

		if (filepath.extension() == ".gtmesh")
			return RequestMesh(id);

		Ref<Asset> asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		mMapMutex.lock();
		if (mRAM.find(id) != mRAM.end())//Already record on map (might still loading)
				asset = mRAM.at(id);
		else//Must load from disk
		{
			mRAM.insert({ id, CreateRef<Asset>(nullptr, id, AssetType::LOADING) });
			LoadFromDisk(id, filepath.string());
			asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		}
		mMapMutex.unlock();
		return asset;
	}

	[[nodiscard]] Ref<Asset> AssetManager::RequestMesh(const uuid& id)
	{
		Ref<Asset> asset = CreateRef<Asset>(nullptr, id, AssetType::INVALID);
		mMapMutex.lock();
		if (mVRAM.find(id) != mVRAM.end())
		{
			if (mVRAM.at(id)->Type == AssetType::MESH)
				asset = mVRAM.at(id);
		}
		else
		{
			let filepath = internal::GetContext()->AssetWatcher.GetFilepath(id);
			Asset raw = internal::Load(filepath);
			asset = CreateRef<Asset>(raw);
			mVRAM.insert({ id, asset });
#ifndef GT_DIST
			GPU::Mesh* geometry = (GPU::Mesh*)raw.Data;
			let& img = geometry->GetThumbnail();
			GPU::Texture2D* texture = GPU::Texture2D::Create(img);
			mThumbnails.insert({ id, CreateRef<Asset>(texture, id, AssetType::TEXTURE, img.Size()) });
#endif
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
			else if (mRAM.at(id)->Type == AssetType::IMAGE)
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
			let filepath = internal::GetContext()->AssetWatcher.GetFilepath(id);
			LoadFromDisk(id, filepath);
			asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		}
		mMapMutex.unlock();
		return asset;
	}

	[[nodiscard]] Ref<Asset> AssetManager::RequestFont(const uuid& id, bool enforcedRAM)
	{
		Ref<Asset> asset = CreateRef<Asset>(nullptr, id, AssetType::INVALID);
		mMapMutex.lock();
		if (mVRAM.find(id) != mVRAM.end() && !enforcedRAM)//Already in GPU
		{
			if (mVRAM.at(id)->Type == AssetType::TEXTURE)
				asset = mVRAM.at(id);
		}
		else if (mRAM.find(id) != mRAM.end())//Cached on RAM
		{
			if (mRAM.at(id)->Type == AssetType::LOADING)//Still loading asset
				asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
			else if (mRAM.at(id)->Type == AssetType::FONT && enforcedRAM)
				asset = mRAM.at(id);
			else if (mRAM.at(id)->Type == AssetType::FONT)
			{
				internal::Font* font = (internal::Font*)mRAM.at(id)->Data;
				GPU::Texture2D* texture = GPU::Texture2D::Create(font->GetAtlas(), gte::ImageFormat::Font);
				asset = CreateRef<Asset>(texture, id, AssetType::TEXTURE, font->GetAtlas().Size());
				mVRAM.insert({ id, asset });
			}
		}
		else
		{
			mRAM.insert({ id, CreateRef<Asset>(nullptr, id, AssetType::LOADING) });
			let filepath = internal::GetContext()->AssetWatcher.GetFilepath(id);
			LoadFromDisk(id, filepath);
			asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		}
		mMapMutex.unlock();
		return asset;
	}

	void AssetManager::LoadFromDisk(const uuid& id, const std::string& filepath)
	{
		//Asynchronous loading
		std::thread handle = std::thread([&](uuid key, std::string filepath) {
			Asset raw = internal::Load(filepath);
			mMapMutex.lock();
			mRAM.at(key) = CreateRef<Asset>(raw);
			mMapMutex.unlock();
			}, id, filepath);
		handle.detach();
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

#ifndef GT_DIST
	Ref<Asset> AssetManager::RequestThumbnail(const uuid& id)
	{
		if (!internal::GetContext()->AssetWatcher.Exists(id))//No asset with such ID exists
			return CreateRef<Asset>(nullptr, id, AssetType::INVALID);
		
		let filepath = std::filesystem::path(internal::GetContext()->AssetWatcher.GetFilepath(id));
		if (filepath.extension() != ".gtmesh" && filepath.extension() != ".gtmat")
			return CreateRef<Asset>(nullptr, id, AssetType::INVALID);


		Ref<Asset> asset = CreateRef<Asset>(nullptr, id, AssetType::LOADING);
		mMapMutex.lock();
		if (mThumbnails.find(id) != mThumbnails.end())
			asset = mThumbnails.at(id);
		else if (filepath.extension() == ".gtmat")
		{
			mMapMutex.unlock();
			Ref<Asset> mat = RequestAsset(id);
			if (mat->Type == AssetType::MATERIAL)
			{
				gte::Material* material = (gte::Material*)mat->Data;
				CreateThumbnail(id, material->img);
			}
			mMapMutex.lock();
		}
		else
		{
			mMapMutex.unlock();
			auto ignore = RequestAsset(id);
			mMapMutex.lock();
		}
		mMapMutex.unlock();
		return asset;
	}

	void AssetManager::CreateThumbnail(const uuid& id, const Image& img)
	{
		std::unique_lock lock(mMapMutex);
		GPU::Texture2D* texture = GPU::Texture2D::Create(img);
		mThumbnails.insert({ id, CreateRef<Asset>(texture, id, AssetType::TEXTURE, img.Size()) });
	}
#endif
}