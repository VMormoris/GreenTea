#include "Font.h"

#include <Engine/Core/Math.h>

#include <yaml-cpp/yaml.h>

namespace gte::internal{


	void Font::Load(const YAML::Node& data)
	{
		using namespace math;
		mCharacters.clear();
		mKernings.clear();

		const auto& glyphs = data["Glyphs"];
		for (const auto& glyph : glyphs)
		{
			uint32 unicode = glyph["Character"].as<uint32>();
			const auto& meta = glyph["Metadata"];
			Character character;
			character.UV = meta["UV"].as<glm::vec4>();
			character.Quad = meta["Quad"].as<glm::vec4>();
			character.Advance = meta["Advance"].as<float>();
			mCharacters.emplace(unicode, character);
		}

		const auto& kernings = data["Kernings"];
		for (const auto& kerning : kernings)
		{
			const std::array<uint32, 2> pair = kerning["Pair"].as<std::array<uint32, 2>>();
			float offset = kerning["Kerning"].as<float>();
			mKernings.emplace(std::make_pair(pair[0], pair[1]), offset);
		}

		const auto& atlas = data["Atlas"];
		uint32 width = atlas["Width"].as<uint32>();
		uint32 height = atlas["Height"].as<uint32>();
		int32 bpp = atlas["Channels"].as<int32>();

		mAtlas = Image(width, height, bpp);
	}

	glm::vec4 Font::GetQuad(uint32 character, uint32 previous) const
	{
		if (mCharacters.find(character) == mCharacters.end())
			return glm::vec4{ 0.0f };

		glm::vec4 quad = mCharacters.at(character).Quad;
		auto key = std::make_pair(character, previous);
		if (mKernings.find(key) != mKernings.end())
			quad.x += mKernings.at(key);

		return quad;
	}

	glm::vec4 Font::GetUV(uint32 character) const
	{
		if (mCharacters.find(character) != mCharacters.end())
			return mCharacters.at(character).UV;
		else
			return glm::vec4{ 0.0f };
	}

	float Font::GetAdvance(uint32 character) const
	{
		if (mCharacters.find(character) != mCharacters.end())
			return mCharacters.at(character).Advance;
		else
			return 0.0f;
	}

	Font::Font(const YAML::Node& data) { Load(data); }

}