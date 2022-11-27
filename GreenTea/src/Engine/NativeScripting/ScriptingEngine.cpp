#include "ScriptingEngine.h"

#include <Engine/Assets/Asset.h>
#include <Engine/Core/Context.h>

#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace gte::internal {

	void ScriptingEngine::Reload(void)
	{
		mEnums.clear();

		if (!std::filesystem::exists(".gt/enums.cache"))
			return;

		YAML::Node enums;
		try { enums = YAML::LoadFile(".gt/enums.cache"); }
		catch(YAML::ParserException e) { ASSERT(false, "Failed to load file: ", "enums.cache", "\n\t", e.what()); }
		for (const auto& enumaration : enums)
		{
			Enum e;
			e.Name = enumaration["Name"].as<std::string>();
			e.Type = (FieldType)enumaration["Type"].as<uint64>();
			ASSERT(isenum(e.Type), "Not a valid type for enumaration");
			const auto& values = enumaration["Values"];
			if (!values)
				continue;
			
			e.Values.reserve(values.size());
			for (const auto& value : values)
			{
				Enum::Value v;
				v.Name = value["Name"].as<std::string>();
				switch (e.Type)
				{
				case FieldType::Enum_Char:
					v.Char = (char)value["Value"].as<int16>();
					break;
				case FieldType::Enum_Int16:
					v.Int16 = value["Value"].as<int16>();
					break;
				case FieldType::Enum_Int32:
					v.Int32 = value["Value"].as<int32>();
					break;
				case FieldType::Enum_Int64:
					v.Int64 = value["Value"].as<int64>();
					break;
				case FieldType::Enum_Byte:
					v.Byte = value["Value"].as<byte>();
					break;
				case FieldType::Enum_Uint16:
					v.Uint16 = value["Value"].as<uint16>();
					break;
				case FieldType::Enum_Uint32:
					v.Uint32 = value["Value"].as<uint32>();
					break;
				case FieldType::Enum_Uint64:
					v.Uint64 = value["Value"].as<int64>();
					break;
				default:
					break;
				}
				e.Values.emplace_back(v);
			}
			mEnums.emplace(e.Name, e);
		}
	}

	ScriptingEngine::ScriptingEngine(void) { Reload(); }

	[[nodiscard]] bool ScriptingEngine::HasEnum(const std::string& name) const noexcept { return mEnums.find(name) != mEnums.end(); }
	
	[[nodiscard]] const Enum& ScriptingEngine::GetEnum(const std::string& name) const
	{
		ASSERT(HasEnum(name), "The specified enum doesn't exist.");
		return mEnums.at(name);
	}

	void ScriptingEngine::Instantiate(void* instance, const NativeScript& description)
	{
		const void* buffer = description.GetBuffer();
		for (const auto& spec : description.GetFieldsSpecification())
		{
			void* dst = (byte*)instance + spec.Offset;
			const void* src = (byte*)buffer + spec.BufferOffset;
			switch (spec.Type)
			{
			case FieldType::Bool:
			case FieldType::Char:
			case FieldType::Int16:
			case FieldType::Int32:
			case FieldType::Int64:
			case FieldType::Byte:
			case FieldType::Uint16:
			case FieldType::Uint32:
			case FieldType::Uint64:
			case FieldType::Enum_Char:
			case FieldType::Enum_Int16:
			case FieldType::Enum_Int32:
			case FieldType::Enum_Int64:
			case FieldType::Enum_Byte:
			case FieldType::Enum_Uint16:
			case FieldType::Enum_Uint32:
			case FieldType::Enum_Uint64:
			case FieldType::Float32:
			case FieldType::Float64:
			case FieldType::Vec2:
			case FieldType::Vec3:
			case FieldType::Vec4:
			case FieldType::Entity:
			//Can be trivially copied
				memcpy(dst, src, spec.Size);
				break;
			case FieldType::String:
			{
				std::string& val = *((std::string*)dst);
				val = *((const std::string*)src);
				break;
			}
			case FieldType::Asset:
			{
				Ref<Asset>* ref = (Ref<Asset>*)dst;
				*ref = internal::GetContext()->AssetManager.RequestAsset((*(const Ref<Asset>*)src)->ID);
				break;
			}
			default:
				break;
			}
		}
	}

}