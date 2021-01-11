#include "Logger.h"

namespace GTE {

	std::ostream* Logger::s_Nlog;
	std::ostream* Logger::s_Elog;
	std::stringstream Logger::s_Buffer;
	std::vector<std::pair<Logger::Type, std::string>> Logger::s_Accumulator;
	char Logger::s_Prefix[23];

	void Logger::Init(std::ostream& output, std::ostream& erroutput) {
		s_Nlog = &output;
		s_Elog = &erroutput;
		s_Prefix[0] = '[';
		s_Prefix[21] = ' ';
		s_Prefix[22] = '\0';
	}

}