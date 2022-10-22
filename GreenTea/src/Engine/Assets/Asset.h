#pragma once
#include <Engine/Core/uuid.h>

namespace gte {

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

		//Script written in C++
		NATIVE_SCRIPT = 4,

		//An Asset containing all Font data on RAM
		FONT = 6,
		//An Asset for Texture Atlas of the font
		//FONT_TEXTURE = 7,

		//A audio file asset
		AUDIO = 8,

		//An animation asset
		ANIMATION = 9,

		//Scene Description
		SCENE = 10,

		PREFAB = 11,
		SHADER = 12
	};

	struct ENGINE_API Asset {
		void* Data = nullptr;
		AssetType Type = AssetType::INVALID;
		uuid ID;
		size_t Size = 0;
		Asset(void) = default;
		Asset(const uuid& id) noexcept
			: ID(id) {}
		Asset(void* data, const uuid& id, AssetType type) noexcept
			: Data(data), ID(id), Type(type) {}
		Asset(void* data, const uuid& id, AssetType type, size_t size) noexcept
			: Data(data), ID(id), Type(type), Size(size) {}

		[[nodiscard]] bool operator==(const Asset& rhs) const noexcept { return ID == rhs.ID; }
		[[nodiscard]] bool operator==(const uuid& rhs) const noexcept { return ID == rhs; }
	};

	namespace internal {
		void Save(const Asset& asset, const ::std::string& filepath);
		Asset Load(const ::std::string& filepath);
	}
}