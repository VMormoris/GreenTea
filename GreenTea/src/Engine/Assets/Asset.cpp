#include "Asset.h"
#include "NativeScript.h"
#include "Image.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace gte::internal {

	Asset Load(const std::string& filepath)
	{
		Asset asset;
		std::ifstream is(filepath, std::ios::binary);
		
		std::string line;
		uint16 loaded = 0;
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
		is.close();

		//Handle buffer depending on AssetType
		switch (asset.Type)
		{
		case AssetType::NATIVE_SCRIPT:
		{
			YAML::Node data;
			try { data = YAML::Load(buffer); }
			catch(YAML::ParserException e) { ASSERT(false, "Failed to load file: ", filepath.c_str(), "\n\t", e.what());}
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
			img->Load((byte*)buffer, asset.Size);
			if (asset.Data)
				delete asset.Data;
			asset.Data = img;
			break;
		}
		case AssetType::MUSIC:
		case AssetType::SOUND:
			//ADD assertion for not implemented yet
			break;
		case AssetType::FONT_IMAGE:
		case AssetType::FONT_TEXTURE:
		case AssetType::TEXTURE:
		case AssetType::LOADING:
		case AssetType::INVALID:
			//ADD assertion for asset type that can't be loaded
			break;
		}
		delete[] buffer;
		return asset;
	}

}