#include "NativeScript.h"

#include <Engine/Assets/Asset.h>
#include <Engine/Core/Math.h>
#include <Engine/Core/Ref.h>
#include <Engine/Scene/Entity.h>

#include <yaml-cpp/yaml.h>

namespace gte::internal {

	void NativeScript::Load(const YAML::Node& data)
	{
		mFieldSpecs.clear();
		mName = data["Name"].as<std::string>();
		mHeader = data["Header"].as<std::string>();
		mVersion = data["Version"].as<uint64>();
		const auto fields = data["Fields"];

		size_t buffOffset = 0;
		for (const auto& meta: fields)
		{
			const auto name = meta["Name"].as<std::string>();
			size_t size = meta["Size"].as<size_t>();
			size_t offset = meta["Offset"].as<size_t>();
			FieldType type = (FieldType)meta["Type"].as<uint64>();
			mFieldSpecs.emplace_back(name, type, size, offset, buffOffset);
			if (isenum(type))
				mFieldSpecs.back().TypeName = meta["TypeName"].as<std::string>();
			buffOffset += size;
		}
		mBufferSize = buffOffset;

		if(mBuffer)//Delete old buffer if exists
			delete[] mBuffer;

		mBuffer = new byte[buffOffset];
		size_t index = 0;
		for (const auto& field: fields)
		{
			auto& spec = mFieldSpecs[index++];
			void* ptr = (void*)(mBuffer + spec.BufferOffset);
			switch(spec.Type)
			{
			case FieldType::Bool:
				*((bool*)ptr) = field["Default"].as<bool>();
				break;
			case FieldType::Char:
				spec.MinInt = field["min"].as<int64>();
				spec.MaxInt = field["max"].as<int64>();
				*((char*)ptr) = (char)field["Default"].as<int16>();
				break;
			case FieldType::Enum_Char:
				*((char*)ptr) = (char)field["Default"].as<int16>();
				break;
			case FieldType::Byte:
				spec.MinUint = field["min"].as<uint64>();
				spec.MaxUint = field["max"].as<uint64>();
				*((byte*)ptr) = (byte)field["Default"].as<uint16>();
				break;
			case FieldType::Enum_Byte:
				*((byte*)ptr) = (byte)field["Default"].as<uint16>();
				break;
			case FieldType::Int16:
				spec.MinInt = field["min"].as<int64>();
				spec.MaxInt = field["max"].as<int64>();
				*((int16*)ptr) = field["Default"].as<int16>();
				break;
			case FieldType::Enum_Int16:
				*((int16*)ptr) = field["Default"].as<int16>();
				break;
			case FieldType::Uint16:
				spec.MinUint = field["min"].as<uint64>();
				spec.MaxUint = field["max"].as<uint64>();
				*((uint16*)ptr) = field["Default"].as<uint16>();
				break;
			case FieldType::Enum_Uint16:
				*((uint16*)ptr) = field["Default"].as<uint16>();
				break;
			case FieldType::Int32:
				spec.MinInt = field["min"].as<int64>();
				spec.MaxInt = field["max"].as<int64>();
				*((int32*)ptr) = field["Default"].as<int32>();
				break;
			case FieldType::Enum_Int32:
				*((int32*)ptr) = field["Default"].as<int32>();
				break;
			case FieldType::Uint32:
				spec.MinUint = field["min"].as<uint64>();
				spec.MaxUint = field["max"].as<uint64>();
				*((uint32*)ptr) = field["Default"].as<uint32>();
				break;
			case FieldType::Enum_Uint32:
				*((uint32*)ptr) = field["Default"].as<uint32>();
				break;
			case FieldType::Int64:
				spec.MinInt = field["min"].as<int64>();
				spec.MaxInt = field["max"].as<int64>();
				*((int64*)ptr) = field["Default"].as<int64>();
				break;
			case FieldType::Enum_Int64:
				*((int64*)ptr) = field["Default"].as<int64>();
				break;
			case FieldType::Uint64:
				spec.MinUint = field["min"].as<uint64>();
				spec.MaxUint = field["max"].as<uint64>();
				*((uint64*)ptr) = field["Default"].as<uint64>();
				break;
			case FieldType::Enum_Uint64:
				*((uint64*)ptr) = field["Default"].as<uint64>();
				break;
			case FieldType::Float32:
				spec.MinFloat = field["min"].as<double>();
				spec.MaxFloat = field["max"].as<double>();
				*((float32*)ptr) = field["Default"].as<float>();
				break;
			case FieldType::Float64:
				spec.MinFloat = field["min"].as<double>();
				spec.MaxFloat = field["max"].as<double>();
				*((float64*)ptr) = field["Default"].as<double>();
				break;
			case FieldType::Vec2:
				spec.MinFloat = field["min"].as<double>();
				spec.MaxFloat = field["max"].as<double>();
				*((glm::vec2*)ptr) = field["Default"].as<glm::vec2>();
				break;
			case FieldType::Vec3:
				spec.MinFloat = field["min"].as<double>();
				spec.MaxFloat = field["max"].as<double>();
				*((glm::vec3*)ptr) = field["Default"].as<glm::vec3>();
				break;
			case FieldType::Vec4:
				spec.MinFloat = field["min"].as<double>();
				spec.MaxFloat = field["max"].as<double>();
				*((glm::vec4*)ptr) = field["Default"].as<glm::vec4>();
				break;
			case FieldType::String:
			{
				spec.Length = field["length"].as<size_t>();
				std::string* temp = new(ptr) std::string();
				*temp = field["Default"].as<std::string>();
				break;
			}
			case FieldType::Asset:
			{
				new(ptr) Ref<Asset>();
				*(Ref<Asset>*)ptr = CreateRef<Asset>();
				break;
			}
			case FieldType::Entity:
				new(ptr) Entity();
				break;
			default:
				ASSERT(false, "Invalid field type.");
				break;
			}
		}
	}

	FieldSpecification::FieldSpecification(const std::string& name, FieldType type, size_t offset, size_t buffOffset) noexcept
		: Name(name), Type(type), Offset(offset), BufferOffset(buffOffset)
	{
		switch (type)
		{
		case FieldType::Bool:
		case FieldType::Char:
		case FieldType::Byte:
		case FieldType::Enum_Char:
		case FieldType::Enum_Byte:
			Size = 1;
			break;
		case FieldType::Int16:
		case FieldType::Uint16:
		case FieldType::Enum_Int16:
		case FieldType::Enum_Uint16:
			Size = 2;
			break;
		case FieldType::Int32:
		case FieldType::Uint32:
		case FieldType::Enum_Int32:
		case FieldType::Enum_Uint32:
		case FieldType::Float32:
			Size = 4;
			break;
		case FieldType::Int64:
		case FieldType::Uint64:
		case FieldType::Enum_Int64:
		case FieldType::Enum_Uint64:
		case FieldType::Float64:
			Size = 8;
			break;
		case FieldType::String:
			Size = sizeof(std::string);
			break;
		case FieldType::Vec2:
			Size = sizeof(float) * 2;
			break;
		case FieldType::Vec3:
			Size = sizeof(float) * 3;
			break;
		case FieldType::Vec4:
			Size = sizeof(float) * 4;
			break;
		case FieldType::Asset:
			Size = sizeof(Ref<Asset>);
			break;
		case FieldType::Entity:
			Size = sizeof(Entity);
			break;
		default:
			ASSERT(false, "Invalid field type.");
			break;
		}
	}

	NativeScript::~NativeScript(void)
	{
		if (mBuffer)
		{
			CleanValues();
			delete[] mBuffer;
		}
		mBuffer = nullptr;
	}

	NativeScript::NativeScript(const NativeScript& other)
	{
		mName = other.mName;
		mVersion = other.mVersion;
		mFieldSpecs = other.mFieldSpecs;
		mBufferSize = other.mBufferSize;
		if (mBuffer)
		{
			CleanValues();
			delete[] mBuffer;
			mBuffer = nullptr;
		}
			
		if (other.mBuffer)
		{
			mBuffer = new byte[other.mBufferSize];
			CopyValues(other);
		}
	}

	NativeScript& NativeScript::operator=(const NativeScript& rhs) noexcept
	{
		if (this == &rhs)
			return *this;
		mName = rhs.mName;
		mVersion = rhs.mVersion;
		mFieldSpecs = rhs.mFieldSpecs;
		mBufferSize = rhs.mBufferSize;
		if (mBuffer)
		{
			CleanValues();
			delete[] mBuffer;
			mBuffer = nullptr;
		}

		if (rhs.mBuffer)
		{
			mBuffer = new byte[rhs.mBufferSize];
			CopyValues(rhs);
		}
		return *this;
	}

	void NativeScript::CleanValues(void)
	{
		for (const auto& spec : mFieldSpecs)
		{
			if (spec.Type != FieldType::Asset && spec.Type != FieldType::String)
				continue;
			void* ptr = mBuffer + spec.BufferOffset;
			if (spec.Type == FieldType::Asset)
			{
				Ref<Asset>& ref = *(Ref<Asset>*)ptr;
				ref.reset();
			}
			else//String
			{
				std::string& ref = *(std::string*)ptr;
				ref.~basic_string();
			}
		}
	}

	void NativeScript::CopyValues(const NativeScript& other)
	{
		for (const auto& spec : other.mFieldSpecs)
		{
			void* ptr = mBuffer + spec.BufferOffset;
			void* optr = other.mBuffer + spec.BufferOffset;
			if (spec.Type == FieldType::Asset)
			{
				new(ptr) Ref<Asset>();
				Ref<Asset>& ref = *(Ref<Asset>*)ptr;
				Ref<Asset>& oref = *(Ref<Asset>*)optr;
				ref = oref;
			}
			else if (spec.Type == FieldType::String)
			{
				new(ptr) std::string();
				std::string& ref = *(std::string*)ptr;
				std::string& oref = *(std::string*)optr;
				ref = oref;
			}
			else
				memcpy(ptr, optr, spec.Size);
		}
	}

	[[nodiscard]] bool isenum(FieldType type) { return type == FieldType::Enum_Char || type == FieldType::Enum_Int16 || type == FieldType::Enum_Int32 || type == FieldType::Enum_Int64 || type == FieldType::Enum_Byte || type == FieldType::Enum_Uint16 || type == FieldType::Enum_Uint32 || type == FieldType::Enum_Uint64; }

}