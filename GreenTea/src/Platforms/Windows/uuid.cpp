#include <Engine/Core/uuid.h>

#include <stdio.h>

//From https://stackoverflow.com/questions/36262070/what-does-htons-do-on-a-big-endian-system
#define FLIP_BYTES_S(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

static constexpr GUID null = { 0 };
static constexpr char nullstr[] = "00000000-0000-0000-0000-000000000000";

namespace gte {

	//From https://stackoverflow.com/questions/18555306/convert-guid-structure-to-lpcstr
	[[nodiscard]] std::string uuid::str(void) const noexcept
	{
		if (mUUID == null)
			return std::string(nullstr);
		char cstr[37] = { 0 };
		sprintf_s
		(
			cstr,//Buffer
			"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",//Format 
			mUUID.Data1,//4 first bytes 
			mUUID.Data2,//2 bytes
			mUUID.Data3,//2 bytes
			mUUID.Data4[0],//2 bytes
			mUUID.Data4[1], mUUID.Data4[2], mUUID.Data4[3], mUUID.Data4[4], mUUID.Data4[5], mUUID.Data4[6], mUUID.Data4[7]//Last 6 bytes
		);
		return std::string(cstr);
	}

	uuid::uuid(const std::string& str) noexcept
	{
		mUUID = { 0 };
		if (str.compare(nullstr) != 0)
		{
			sscanf_s(str.c_str(), "%08x", &mUUID.Data1);
			sscanf_s(str.c_str() + 9, "%04hx", &mUUID.Data2);
			sscanf_s(str.c_str() + 14, "%04hx", &mUUID.Data3);

			unsigned short* ptr = (unsigned short*)&mUUID.Data4;
			sscanf_s(str.c_str() + 19, "%04hx", ptr++);
			sscanf_s(str.c_str() + 24, "%04hx", ptr++);
			sscanf_s(str.c_str() + 28, "%04hx", ptr++);
			sscanf_s(str.c_str() + 32, "%04hx", ptr);

			ptr -= 3;
			for (size_t i = 0; i < 4; i++)
			{
				*ptr = FLIP_BYTES_S(*ptr);
				ptr++;
			}
		}
	}

	//TODO(Vasilis): Add assertion
	[[nodiscard]] uuid uuid::Create(void) noexcept
	{
		uuid newone;
		HRESULT hr = CoCreateGuid(&newone.mUUID);
		return newone;
	}

	uuid::uuid(void) noexcept { memset(&mUUID, 0, sizeof(GUID)); }
	[[nodiscard]] bool uuid::operator==(const uuid& rhs) const noexcept { return memcmp(&mUUID, &rhs.mUUID, sizeof(GUID)) == 0; }
	[[nodiscard]] bool uuid::operator!=(const uuid& rhs) const noexcept { return memcmp(&mUUID, &rhs.mUUID, sizeof(GUID)) != 0; }
	[[nodiscard]] bool uuid::IsValid(void) const noexcept { return memcmp(&mUUID, &null, sizeof(GUID)) != 0; }

}

[[nodiscard]] std::ostream& operator<<(std::ostream& lhs, const gte::uuid& rhs)
{
	lhs << rhs.str();
	return lhs;
}

namespace std {
	
	[[nodiscard]] size_t hash<gte::uuid>::operator()(const gte::uuid& id) const
	{
		RPC_STATUS status = RPC_S_OK;
		unsigned short value = UuidHash(&const_cast<GUID&>(id.mUUID), &status);
		//TODO(Vasilis): Add assertion
		return std::hash<unsigned short>()(value);
	}

}