#pragma once

#include "Image.h"

#include <glm.hpp>
#include <map>

//Forward Decleration
namespace YAML { class Node; }

namespace gte::internal {

	struct ENGINE_API Character {
		glm::vec4 UV{};
		glm::vec4 Quad{};
		float Advance = 0.0f;
	};

	class ENGINE_API Font {
	public:
		Font(void) = default;
		Font(const YAML::Node& data);
		void Load(const YAML::Node& data);

		glm::vec4 GetQuad(uint32 character, uint32 previous = 0) const;
		glm::vec4 GetUV(uint32 character) const;
		float GetAdvance(uint32 character) const;

		void SetCharacters(const std::map<uint32, Character>& characters) noexcept { mCharacters = characters; }
		const std::map<uint32, Character>& GetCharacters(void) const noexcept { return mCharacters; }
		void SetAtlas(const Image& atlas) noexcept { mAtlas = atlas; }
		Image& GetAtlas(void) noexcept { return mAtlas; }
		const Image& GetAtlas(void) const noexcept { return mAtlas; }
		void SetKernings(const std::map<std::pair<uint32, uint32>, float> kernings) noexcept { mKernings = kernings; }
		const std::map<std::pair<uint32, uint32>, float>& GetKernings(void) const noexcept { return mKernings; }

	private:
		std::map<std::pair<uint32, uint32>, float> mKernings;
		std::map<uint32, Character> mCharacters;
		Image mAtlas;
	};

}