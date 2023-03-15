#pragma once

#include <Engine/Core/Engine.h>
#include <string>
#include <vector>

//Forward Decleration
namespace YAML { class Node; }

namespace gte::internal {

	/**
	* @brief Enumaration for every kind of type that a
	*	reflective field can possibly have.
	*/
	enum class ENGINE_API FieldType : byte {
		Unknown = 0,

		Bool,
		Char,
		Byte,//unsigned char

		Int16,
		Int32,
		Int64,
		Uint16,
		Uint32,
		Uint64,

		Float32,
		Float64,


		//Vectors
		Vec2,
		Vec3,
		Vec4,

		//Enumerations
		Enum_Char,
		Enum_Byte,
		Enum_Int16,
		Enum_Int32,
		Enum_Int64,
		Enum_Uint16,
		Enum_Uint32,
		Enum_Uint64,

		String,

		//Engine's Objects
		Asset,
		Entity
	};

	ENGINE_API [[nodiscard]] bool isenum(FieldType type);

	enum class ENGINE_API ReflectionType : byte {
		Unknown = 0,
		Enumaration,
		Component,
		System,
		Object,
		Property,
		Method,
	};

	/**
	* @brief Metadata for hanlding Fields
	*/
	struct ENGINE_API FieldSpecification {
		std::string Name = "Unnamed Field";
		FieldType Type = FieldType::Unknown;
		std::string TypeName = "";
		size_t Size = 0;
		size_t Offset = 0;
		size_t BufferOffset = 0;

		union {
			size_t Length = 0;
			int64 MinInt;
			uint64 MinUint;
			float64 MinFloat;
		};

		union {
			int64 MaxInt = 0;
			uint64 MaxUint;
			float64 MaxFloat;
		};

		FieldSpecification(void) = default;
		FieldSpecification(const std::string& name, FieldType type, size_t offset, size_t buffOffset);
		FieldSpecification(const std::string& name, FieldType type, size_t size, size_t offset, size_t buffOffset) noexcept
			: Name(name), Type(type), Size(size), Offset(offset), BufferOffset(buffOffset) {}
	};

	/**
	* @brief Object that describes a Native Script written by a user (is basically our Asset)
	*/
	class ENGINE_API NativeScript {
	public:
		NativeScript(void) = default;
		NativeScript(const NativeScript& other);
		~NativeScript(void);

		void Load(const YAML::Node& data);

		[[nodiscard]] std::string& GetName(void) noexcept { return mName; }
		[[nodiscard]] const std::string& GetName(void) const noexcept { return mName; }
		[[nodiscard]] std::string& GetHeader(void) noexcept { return mHeader; }
		[[nodiscard]] const std::string& GetHeader(void) const noexcept { return mHeader; }

		void SetVersion(uint64 version) noexcept { mVersion = version; }
		[[nodiscard]] uint64 GetVersion(void) const noexcept { return mVersion; }

		[[nodiscard]] size_t GetBufferSize(void) const noexcept { return mBufferSize; }
		[[nodiscard]] void* GetBuffer(void) noexcept { return mBuffer; }
		[[nodiscard]] const void* GetBuffer(void) const noexcept { return mBuffer; }
		
		[[nodiscard]] const std::vector<FieldSpecification>& GetFieldsSpecification(void) const noexcept { return mFieldSpecs; }
		[[nodiscard]] std::vector<FieldSpecification>& GetFieldsSpecification(void) noexcept { return mFieldSpecs; }

		NativeScript& operator=(const NativeScript& rhs) noexcept;

	private:

		void CleanValues(void);
		void CopyValues(const NativeScript& other);

	private:
		//TODO(Vasilis): Maybe also add type
		std::string mName = "";
		std::string mHeader = "";
		uint64 mVersion = 0;
		std::vector<FieldSpecification> mFieldSpecs;
		size_t mBufferSize = 0;
		byte* mBuffer = nullptr;//Holds the defaults values
	};
}