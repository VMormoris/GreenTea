#include "Asset.h"
#include "Animation.h"
#include "Font.h"
#include "Image.h"
#include "Material.h"
#include "NativeScript.h"
#include "Prefab.h"

#include <Engine/Audio/AudioBuffer.h>
#include <Engine/Core/Math.h>
#include <Engine/GPU/Mesh.h>

#include <fstream>
#include <yaml-cpp/yaml.h>

static gte::Material* LoadMaterial(const YAML::Node& data, std::ifstream& is);

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
		case AssetType::SPRITE_ANIMATION:
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
		case AssetType::MATERIAL:
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
			asset.Data = LoadMaterial(data, is);
			break;
		}
		case AssetType::MESH:
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
			asset.Data = new GPU::Mesh(data, is);
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

gte::Material* LoadMaterial(const YAML::Node& data, std::ifstream& ifs)
{
	using namespace gte::math;
	gte::Material* material = new gte::Material();
	material->Name = data["Name"].as<std::string>();

	material->Albedo->ID = data["Albedo"].as<std::string>();
	material->Metallic->ID = data["Metallic"].as<std::string>();
	material->Rough->ID = data["Roughness"].as<std::string>();
	material->Normal->ID = data["Normal"].as<std::string>();
	material->AmbientOclussion->ID = data["AmbientOclussion"].as<std::string>();
	material->Opacity->ID = data["Opacity"].as<std::string>();
	material->Emission->ID = data["Emission"].as<std::string>();

	material->Diffuse = data["Diffuse"].as<glm::vec4>();
	material->EmitColor = data["EmitColor"].as<glm::vec4>();
	material->AmbientColor = data["AmbientColor"].as<glm::vec4>();
	material->Metallicness = data["Metallicness"].as<float>();
	material->Roughness = data["Roughness"].as<float>();
	material->Alpha = data["Alpha"].as<float>();
	material->IlluminationModel = data["IlluminationModel"].as<int32>();
	material->IsEmissive = data["IsEmissive"].as<bool>();

#ifndef GT_DIST
	uint32 width, height;
	int32 bpp;
	ifs.read((char*)&width, 4);
	ifs.read((char*)&height, 4);
	ifs.read((char*)&bpp, 4);
	material->img = gte::Image(width, height, bpp);
	ifs.read((char*)material->img.Data(), material->img.Size());
#endif
	return material;
}