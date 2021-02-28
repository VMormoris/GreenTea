#include "Material.h"

namespace GTE {

	Material::Material(void)
	{
		Ref<Asset> DiffuseTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		Ref<Asset> SpecularTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		Ref<Asset> AmbientTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		Ref<Asset> NormalTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		Ref<Asset> BumpTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		Ref<Asset> SpecularityTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		Ref<Asset> OpacityTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
	}

}