#include <Engine/Core/uuid.h>

#include <stdio.h>
#include <cstring>


static constexpr uuid_t null = { 0 };
static constexpr char nullstr[] = "00000000-0000-0000-0000-000000000000";

namespace gte {

	[[nodiscard]] std::string uuid::str(void) const noexcept
	{
		char cstr[37] = { 0 };
		uuid_unparse(mUUID, cstr);
		return std::string(cstr);
	}

	uuid::uuid(const std::string& str) noexcept { uuid_parse(str.c_str(), mUUID); }

	//TODO(Vasilis): Add assertion
	[[nodiscard]] uuid uuid::Create(void) noexcept
	{
		uuid newone;
		uuid_generate(newone.mUUID);
		return newone;
	}

	uuid::uuid(void) noexcept { memset(&mUUID, 0, sizeof(uuid_t)); }
	[[nodiscard]] bool uuid::operator==(const uuid& rhs) const noexcept { return memcmp(&mUUID, &rhs.mUUID, sizeof(uuid_t)) == 0; }
	[[nodiscard]] bool uuid::operator!=(const uuid& rhs) const noexcept { return memcmp(&mUUID, &rhs.mUUID, sizeof(uuid_t)) != 0; }
	[[nodiscard]] bool uuid::IsValid(void) const noexcept { return memcmp(&mUUID, &null, sizeof(uuid_t)) != 0; }

}

[[nodiscard]] std::ostream& operator<<(std::ostream& lhs, const gte::uuid& rhs)
{
	lhs << rhs.str();
	return lhs;
}

namespace std {

	//TODO(Vasilis): Find something better instead of using string
	[[nodiscard]] size_t hash<gte::uuid>::operator()(const gte::uuid& id) const { return std::hash<std::string>()(id.str()); }

}