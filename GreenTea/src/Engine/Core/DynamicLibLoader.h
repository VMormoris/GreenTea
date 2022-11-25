#pragma once

#include "Engine.h"

#include <unordered_map>

namespace gte::internal {

	/**
	* @brief Helper class for runtime-loading of game's logic dll
	* @warning One Loader per Library
	*/
	class ENGINE_API DynamicLibLoader {
		typedef void* (*CreateFunc)(void);
	public:

		/**
		* @briefs Constructs a Loader
		*/
		DynamicLibLoader(void);

		/**
		* @brief Loads a shared library at runtime
		* @param filepath Path to file that corresponds to the Library
		* @returns True if the libary loading was sucessfull, false otherwise
		*/
		bool Load(const char* filepath);

		/**
		* @brief Tries to Hot-Reload the last loaded library if there is one
		* @returns True if the libary loading was sucessfull, false otherwise
		* @warning In case there is no library already loaded this function will always return false
		*/
		bool Reload(void);

		/**
		* @brief Tries to unload the last loaded library if there is one
		*/
		void Unload(void);

		/**
		* @brief Creates a new Instance of Class from a Dunamic Library Linked at Runtime
		* @tparam T Type of the Class this parameter is usually ScriptableEntity
		* @param type Type of the actual instance as a String
		* @returns A pointer to the newly created instance.
		* @warning This helper function is meant to be used only for Scripting Classes
		*/
		template<typename T>
		T* CreateInstance(const std::string& type);

	private:

		/**
		* @brief Handle to Library that is being loaded
		*/
		void* mHdll;

		/**
		* @brief Filepath to the loaded library
		*/
		char mLibsFilePath[512];

		/**
		* @brief Map for creation function for each type
		*/
		std::unordered_map<std::string, CreateFunc> mCreateFunctions;

	};

}

#ifdef PLATFORM_WINDOWS
	#include <Platforms/Windows/DynamicLibLoader.hpp>
#else
	#include <Platforms/Linux/DynamicLibLoader.hpp>
#endif
