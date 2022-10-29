#include "Asset.h"
#include "Image.h"
#include "NativeScript.h"
#include "Prefab.h"

#include <Engine/Assets/Font.h>
#include <Engine/Assets/Animation.h>
#include <Engine/Audio/AudioBuffer.h>

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace gte::internal {

	Asset Load(const std::string& filepath)
	{
		Asset asset;
		std::ifstream is(filepath, std::ios::binary);

		uint16 loaded = 0;
		std::string line;
		while (getline(is, line))
		{
			if (line[0] == '#')//Comment ingore
				continue;
			else if (line.empty() || line[0] == '\n' || line[0] == '\r')
				break;
			if (loaded == 0)
				asset.Type = (AssetType)std::stoul(line);
			else if (loaded == 1)
				asset.ID = line;
			else if (loaded == 2)
				asset.Size = std::stoull(line);
			loaded++;
		}

		char* buffer = new char[asset.Size+1];
		buffer[asset.Size] = 0;
		is.read(buffer, asset.Size);
		
		//Handle buffer depending on AssetType
		switch (asset.Type)
		{
		case AssetType::NATIVE_SCRIPT:
		{
			YAML::Node data;
			try { data = YAML::Load(buffer); }
			catch (YAML::ParserException e)
			{
				GTE_ERROR_LOG(false, "Failed to load file: ", filepath, "\n\t", e.what());
				asset.Type = AssetType::INVALID;
				asset.Size = 0;
				asset.ID = {};
				delete[] buffer;
				return asset;
			}
			internal::NativeScript* script = new NativeScript();
			script->Load(data);
			if (asset.Data)
				delete asset.Data;
			asset.Data = script;
			break;
		}
		case AssetType::IMAGE:
		{
			Image* img = new Image();
			img->Load((byte*)buffer);
			if (asset.Data)
				delete asset.Data;
			asset.Data = img;
			break;
		}
		case AssetType::AUDIO:
		{
			int32 format;
			int32 samplerate;
			memcpy(&format, buffer, sizeof(int32));
			memcpy(&samplerate, buffer + 4, sizeof(int32));
			asset.Data = new audio::AudioBuffer(buffer + 8, asset.Size - 8, format, samplerate);
			break;
		}
		case AssetType::PREFAB:
		{
			YAML::Node data;
			try { data = YAML::Load(buffer); }
			catch (YAML::ParserException e)
			{
				GTE_ERROR_LOG(false, "Failed to load file: ", filepath, "\n\t", e.what());
				asset.Type = AssetType::INVALID;
				asset.Size = 0;
				asset.ID = {};
				delete[] buffer;
				return asset;
			}
			asset.Data = new Prefab(data);
			break;
		}
		case AssetType::FONT:
		{
			YAML::Node data;
			try { data = YAML::Load(buffer); }
			catch (YAML::ParserException e)
			{
				GTE_ERROR_LOG(false, "Failed to load file: ", filepath, "\n\t", e.what());
				asset.Type = AssetType::INVALID;
				asset.Size = 0;
				asset.ID = {};
				delete[] buffer;
				return asset;
			}
			char newLine;
			is.read(&newLine, 1);
			internal::Font* font = new internal::Font(data);
			Image& atlas = font->GetAtlas();
			is.read((char*)atlas.Data(), atlas.Size());
			asset.Data = font;
			break;
		}
		case AssetType::ANIMATION:
		{
			if (!asset.Size) { asset.Data = new Animation(); break; }
			
			YAML::Node data;
			try { data = YAML::Load(buffer); }
			catch (YAML::ParserException e)
			{
				GTE_ERROR_LOG(false, "Failed to load file: ", filepath, "\n\t", e.what());
				asset.Type = AssetType::INVALID;
				asset.Size = 0;
				asset.ID = {};
				delete[] buffer;
				return asset;
			}

			Animation* animation = new Animation(data);
			animation->GetID() = asset.ID;
			asset.Data = animation;

			break;
		}
		case AssetType::TEXTURE:
		case AssetType::LOADING:
		case AssetType::INVALID:
			//ADD assertion for asset type that can't be loaded
			break;
		}
		is.close();
		delete[] buffer;
		return asset;
	}

}