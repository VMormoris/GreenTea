#include "Logger.h"
#include "Context.h"

namespace gte::internal {

	[[nodiscard]] std::string Logger::type_tostr(Type logtype) noexcept
	{
		switch (logtype)
		{
			case Type::TRACE:	return "(TRACE): ";
			case Type::INFO:	return "(INFO): ";
			case Type::WARNING: return "(WARNING): ";
			case Type::ERR:		return "(ERROR): ";
			case Type::FATAL:	return "(FATAL ERROR): ";
			default:			return "";//Unreachable
		}
	}

	[[nodiscard]] Logger* Logger::Get(void) noexcept { return &GetContext()->logger; }

	void Logger::WriteToFile(void)
	{
		mWrittingToFile = true;
		std::ofstream logfile("record.gtlog");
	}

}