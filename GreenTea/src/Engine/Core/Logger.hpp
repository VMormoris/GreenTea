#pragma once
#include "Logger.h"
#include <iostream>

namespace gte::internal {

	template<typename ...Arguments>
	inline void Logger::Log(Type logtype, Arguments&& ...args)
	{
		//Create subfix
		std::string subfix = type_tostr(logtype);

		//empty buffer
		mBuffer.clear();
		mBuffer.str(std::string());

		//Prefix buffer for time (format: [DD/MM/YYYY HH:MM:SS])
		time_t raw_time = time(NULL);
		struct tm tm_buf;
#ifdef PLATFORM_WINDOWS
		localtime_s(&tm_buf, &raw_time);
#else
		localtime_r(&raw_time, &tm_buf);
#endif
		strftime(mPrefix + sizeof(char), 20, "%d-%m-%Y %H:%M:%S", &tm_buf);
		mPrefix[20] = ']';

		mBuffer << mPrefix << subfix;

		//fill buffer with new data
		addToBuffer(args...);
		mBuffer << '\n';

		mAccumulator.push_back(std::make_pair(logtype, mBuffer.str()));

		if (logtype == Type::ERR || logtype == Type::FATAL) {
			std::cerr << mBuffer.str();
			if (logtype == Type::FATAL) exit(EXIT_FAILURE);
		}
		else
			std::cout << mBuffer.str();
	}

	template<typename First, typename ...Rest>
	inline void Logger::addToBuffer(First&& arg, Rest&& ...args)
	{
		addToBuffer(arg);
		addToBuffer(args...);
	}
}