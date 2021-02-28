#ifndef _LOGGER
#define _LOGGER

#include "EngineCore.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>

namespace GTE {

	/**
	* @brief Class for Logging Any amount of Arguments
	* @details Logging will be visible in the Console Log Panel as well as the desired outputstreams
	* @warning for each one of the arguments std::stringstream& operator<<(std::stringstream& ss, Arugment arg) MUST BE DEFINED
	*/
	class ENGINE_API Logger {
	public:

		/**
		* @brief Enumaration for the severity level of the Log message
		*/
		enum class Type {
			/**
			* @brief Used for messages that usually show progress
			*/
			TRACE = 0,
			/**
			* @brief Used for displaying information to the developer
			*/
			INFO,
			/**
			* @brief Used for warning the developer
			*/
			WARNING,
			/**
			* @brief Used when error is occuried
			*/
			ERR,
			/**
			* @brief Used when the occuried error is severe
			*/
			FATAL
		};

	public:

		/**
		* @brief Getter for the history of Logs
		* @returns A vector of pairs, containing the Log's severity level and the message
		*/
		static std::vector<std::pair<Type, std::string>>& Get() { return s_Accumulator; }

		/**
		* @brief Clears the Logs' history
		*/
		static void Clear(void) { s_Accumulator.clear(); }

		/**
		* @brief Logging any amount of arguments
		* @details Logging Format: [DD/MM/YYYY HH:MM:SS] [File: filename, line: line] (logtype): Arguments...
		* @tparam Arguments Types for each one of the Arguments that will be logged
		* @param logtype Severity level of the Logging message
		* @param file Filename that the function is called from
		* @param line Line in the file where the Function is called from
		* @param args The Arguments to be Logged
		*/
		template<typename ...Arguments>
		static void Log(Type logtype, const char* file, int32 line, Arguments ...args)
		{
			//Create subfix
			std::string subfix = create_subfix(logtype, file, line);

			//empty buffer
			s_Buffer.clear();
			s_Buffer.str(std::string());

			//Prefix buffer for time (format: [DD/MM/YYYY HH:MM:SS])
			time_t raw_time= time(NULL);
			struct tm tm_buf;
			localtime_s(&tm_buf, &raw_time);
			strftime(s_Prefix + sizeof(char), 20, "%d-%m-%Y %H:%M:%S", &tm_buf);
			s_Prefix[20] = ']';

			s_Buffer << s_Prefix << subfix;

			//fill buffer with new data
			addToBuffer(args...);
			s_Buffer << '\n';

			s_Accumulator.push_back(std::make_pair(logtype, s_Buffer.str()));

			if (logtype == Type::ERR || logtype == Type::FATAL) {
				*s_Elog << s_Buffer.str();
				if (logtype == Type::FATAL) exit(EXIT_FAILURE);
			}
			else
				*s_Nlog << s_Buffer.str();

		}

		/**
		* @brief Logger Initialization
		* @param output Output stream to be used as output for (Trace, Info and Warning logs)
		* @param erroutput Output stream to be used as output for (Error and Fatal Error logs)
		*/
		static void Init(std::ostream& output=std::cout, std::ostream& erroutput=std::cerr);
		
	private:

		/**
		* @brief Add a single Element to stream
		* @tparam T Type of the argument
		* @param data Argument to be added on the stream
		*/
		template<typename T>
		static void addToBuffer(T data) { s_Buffer << data; }
		
		/**
		* @brief Recursively add one element to stream
		* @tparam First Type of the first argument to be added to stream
		* @tparam Rest Types for each one of all the others arguments to be added to the stream
		* @param arg First Argument to be added to the stream
		* @param args The Rest of the Arguments that will be added recusively
		*/
		template<typename First, typename ...Rest>
		static void addToBuffer(First arg, Rest ... args) {
			addToBuffer(arg);
			addToBuffer(args...);
		}
		
		/**
		* @brief Helper funnction for constructing the Log in human readable form
		*/
		static std::string create_subfix(Type logtype, const char* filename, int32 line)
		{
			std::stringstream stream;
			stream.str(std::string());
			stream << "[File: " << utils::strip_path(filename) << ", line: " << line << "]";
			
			if (logtype == Type::TRACE) stream << " (TRACE): ";
			else if (logtype == Type::INFO) stream << " (INFO): ";
			else if (logtype == Type::WARNING) stream << " (WARNING): ";
			else if (logtype == Type::ERR) stream << " (ERROR): ";
			else if (logtype == Type::FATAL) stream << " (FATAL ERROR): ";
			return stream.str();
		}
	
		/**
		* @brief Temporary buffer for the Argument to be logged
		*/
		static std::stringstream s_Buffer;
		
		/**
		* @brief Output stream for (Debug, Ingo and Warning logs)
		*/
		static std::ostream* s_Nlog;
		
		/**
		* @brief Output stream for (Error and Fatal Error logs)
		*/
		static std::ostream* s_Elog;

		/**
		* @brief C-style string containing the time of each new log
		*/
		static char s_Prefix[23];

		/**
		* @brief Logs' history
		* @details List containing all the logs
		*/
		static std::vector<std::pair<Type, std::string>> s_Accumulator;

	};

}

/**
* @brief Trace LOG
*/
#define GTE_TRACE_LOG(...) GTE::Logger::Log(GTE::Logger::Type::TRACE, __FILE__, __LINE__, __VA_ARGS__)
/**
* @brief Information Log
*/
#define GTE_INFO_LOG(...) GTE::Logger::Log(GTE::Logger::Type::INFO, __FILE__, __LINE__, __VA_ARGS__)
/**
* @brief Warning Log
*/
#define GTE_WARN_LOG(...) GTE::Logger::Log(GTE::Logger::Type::WARNING, __FILE__, __LINE__, __VA_ARGS__)
/**
* @brief Error Log
*/
#define GTE_ERROR_LOG(...) GTE::Logger::Log(GTE::Logger::Type::ERR, __FILE__, __LINE__, __VA_ARGS__)
/**
* @brief Fatal Error Log
*/
#define GTE_FATAL_LOG(...) GTE::Logger::Log(GTE::Logger::Type::FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif