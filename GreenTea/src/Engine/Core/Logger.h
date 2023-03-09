#pragma once

#include "Engine.h"
#include <sstream>
#include <vector>

namespace gte::internal {

	/**
	* @brief Singleton for Logging Any amount of Arguments
	* @details Logging will be visible in the Console Log Panel as well as the desired outputstreams
	* @warning for each one of the arguments std::stringstream& operator<<(std::stringstream& ss, Arugment arg) MUST BE DEFINED
	*/
	class ENGINE_API Logger {
	public:
		
		/**
		* @brief Enumaration describing the severity level of the Log message
		*/
		enum class Type : byte {
			/**
			* @brief Not valid type used for filters
			*/
			NONE = 0x00,
			/**
			* @brief Used for messages that usually show progress
			*/
			TRACE = 0x01,
			/**
			* @brief Used for displaying information to the developer
			*/
			INFO = 0x02,
			/**
			* @brief Used for warning the developer
			*/
			WARNING = 0x04,
			/**
			* @brief Used when error is occuried
			*/
			ERR = 0x08,
			/**
			* @brief Used when the occuried error is severe
			*/
			FATAL = 0x10
		};

	public:

		/**
		* @brief Clears the Logs' history
		*/
		void Clear(void) noexcept { mAccumulator.clear(); }

		/**
		* @brief Get the Logs' history
		* @return The accumulated logs
		*/
		[[nodiscard]] const std::vector<std::pair<Type, std::string>>& GetHistory(void) noexcept { return mAccumulator; }

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
		void Log(Type logtype, Arguments&& ...args);

		/**
		* @brief Gives access to the single instance of the Logger object
		* @return Pinter to the singleton instance
		*/
		[[nodiscard]] static Logger* Get(void) noexcept;

		void WriteToFile(void);

	private:

		template<typename T>
		void addToBuffer(T data) { mBuffer << data; }

		template<typename First, typename ...Rest>
		void addToBuffer(First&& arg, Rest&& ...args);

		[[nodiscard]] std::string type_tostr(Type logtype) noexcept;

	private:
		char mPrefix[23] = "\0";
		std::stringstream mBuffer;
		std::vector<std::pair<Type, std::string>> mAccumulator;
		bool mWrittingToFile = false;
	};

	ENGINE_API [[nodiscard]] inline Logger::Type operator|(Logger::Type lhs, Logger::Type rhs) noexcept { return static_cast<Logger::Type>(static_cast<byte>(lhs) | static_cast<byte>(rhs)); }
	ENGINE_API [[nodiscard]] inline Logger::Type& operator|=(Logger::Type& lhs, Logger::Type rhs) noexcept { return lhs = lhs | rhs; }
	ENGINE_API [[nodiscard]] inline Logger::Type operator&(Logger::Type lhs, Logger::Type rhs) noexcept { return static_cast<Logger::Type>(static_cast<byte>(lhs) & static_cast<byte>(rhs)); }
	ENGINE_API [[nodiscard]] inline Logger::Type& operator&=(Logger::Type& lhs, Logger::Type rhs) noexcept { return lhs = lhs & rhs; }
	ENGINE_API [[nodiscard]] inline Logger::Type operator~(Logger::Type rhs) noexcept { return static_cast<Logger::Type>(~static_cast<byte>(rhs)); }
}

/**
* @brief Trace LOG
*/
#define GTE_TRACE_LOG(...) gte::internal::Logger::Get()->Log(gte::internal::Logger::Type::TRACE, __VA_ARGS__)
/**
* @brief Information Log
*/
#define GTE_INFO_LOG(...) gte::internal::Logger::Get()->Log(gte::internal::Logger::Type::INFO, __VA_ARGS__)
/**
* @brief Warning Log
*/
#define GTE_WARN_LOG(...) gte::internal::Logger::Get()->Log(gte::internal::Logger::Type::WARNING, __VA_ARGS__)
/**
* @brief Error Log
*/
#define GTE_ERROR_LOG(...) gte::internal::Logger::Get()->Log(gte::internal::Logger::Type::ERR, __VA_ARGS__)
/**
* @brief Fatal Error Log
*/
#define GTE_FATAL_LOG(...) gte::internal::Logger::Get()->Log(gte::internal::Logger::Type::FATAL, __VA_ARGS__)

#include "Logger.hpp"