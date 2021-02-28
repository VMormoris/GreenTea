#ifndef _UTILS
#define _UTILS

#include "EngineCore.h"
#include <string>

namespace GTE::utils {

	/**
	* @brief Strips the path from a filepath string
	* @param filepath Filepath to strip path from
	* @returns A string containing the file's name with the path stripped
	*/
	ENGINE_API std::string strip_path(const std::string& filepath);

	/**
	* @brief Returns the folder's path of the given filepath string
	* @param filepath Filepath to get the path from
	* @returns A string containing the folder's path
	*/
	ENGINE_API std::string get_path(const std::string& filepath);

	/**
	* @brief Reads a text file
	* @param filepath C-String that contains the filepath
	* @returns A C-String containing the contents of the File
	* @warning Use delete[] operator to dealocate the memory for the contents of the file
	*/
	ENGINE_API const char* readfile(const char* filepath);


	/**
	* @brief Writes a text file
	* @param filepath C-String that contains the filepath
	* @param str C-String containing the contents that would be written to the file
	*/
	ENGINE_API void writefile(const char* filepath, const std::string& str);

	/**
	* @brief Converts a string into a new one with letters being lower case
	* @param str The string to be converted into lower case
	* @returns The new string that contains only lower case characters
	*/
	std::string tolowerCase(const std::string& str);

	/**
	* @brief Compares two string with no case sensitivity
	* @param left The first string to be compared
	* @param right The second strin to be compared
	* @returns True if the string are the same false otherwise
	*/
	ENGINE_API bool compareStringIgnoreCase(const std::string& left, const std::string& right);

}
#endif