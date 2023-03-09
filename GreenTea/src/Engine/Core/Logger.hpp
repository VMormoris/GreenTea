#pragma once
#include "Logger.h"

#include <iostream>
#include <fstream>

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
		localtime_s(&tm_buf, &raw_time);
		strftime(mPrefix + sizeof(char), 20, "%d-%m-%Y %H:%M:%S", &tm_buf);
		mPrefix[20] = ']';

		mBuffer << mPrefix << subfix;

		//fill buffer with new data
		addToBuffer(args...);
		mBuffer << '\n';

		mAccumulator.push_back(std::make_pair(logtype, mBuffer.str()));

		if (logtype == Type::ERR || logtype == Type::FATAL) {
			if (mWrittingToFile)
			{
				std::ofstream logfile("record.gtlog", std::ios::app | std::ios::binary);
				logfile << mBuffer.str();
			}
			else
				std::cerr << mBuffer.str();
			if (logtype == Type::FATAL) exit(EXIT_FAILURE);
		}
		else
		{
			if (mWrittingToFile)
			{
				std::ofstream logfile("record.gtlog", std::ios::app | std::ios::binary);
				logfile << mBuffer.str();
			}
			else
				std::cout << mBuffer.str();
		}
			
	}

	template<typename First, typename ...Rest>
	inline void Logger::addToBuffer(First&& arg, Rest&& ...args)
	{
		addToBuffer(arg);
		addToBuffer(args...);
	}
}