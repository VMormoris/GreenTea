#pragma once
#include "System.h"

#include <Engine/Assets/NativeScript.h>
#include <Engine/Core/uuid.h>

#include <map>

namespace gte::internal {

	struct ENGINE_API Enum {

		struct Value {
			std::string Name;
			union {
				char Char;
				int16 Int16;
				int32 Int32;
				int64 Int64;

				byte Byte;
				uint16 Uint16;
				uint32 Uint32;
				uint64 Uint64 = 0;
			};
		};

		std::string Name;
		FieldType Type = FieldType::Unknown;
		std::vector<Value> Values;
	};

	class ENGINE_API ScriptingEngine {
	public:
		ScriptingEngine(void);
		void Reload(void);

		[[nodiscard]] bool HasEnum(const std::string& name) const noexcept;
		[[nodiscard]] const Enum& GetEnum(const std::string& name) const;

		void Instantiate(void* instance, const NativeScript& description);

	private:
		std::map<std::string, Enum> mEnums;
	};

}