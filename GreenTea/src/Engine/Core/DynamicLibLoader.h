#pragma once

#include "Engine.h"

#include <unordered_map>

namespace gte {

	//Forward declaration(s)
	class Entity;
	
	namespace internal {

		/**
		* @brief Helper class for runtime-loading of game's logic dll
		* @warning One Loader per Library
		*/
		class ENGINE_API DynamicLibLoader {
			typedef void* (*CreateFunc)(void);
			typedef bool (*HasCompFunc)(Entity);
			typedef void* (*EmplaceOrReplaceCompFunc)(Entity);
			typedef void (*RemoveCompFunc)(Entity);
		public:

			/**
			* @briefs Constructs a Loader
			*/
			DynamicLibLoader(void);

			/**
			* @brief Loads a shared library at runtime
			* @param filepath Path to file that corresponds to the Library
			* @return True if the libary loading was sucessfull, false otherwise
			*/
			bool Load(const char* filepath);

			/**
			* @brief Sets the shared library that will be loaded at a later time
			* @param filepath Path to file that corresponds to the Library
			*/
			void SetLibFile(const char* filepath);

			/**
			* @brief Tries to Hot-Reload the last loaded library if there is one
			* @return True if the libary loading was sucessfull, false otherwise
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
			* @return A pointer to the newly created instance.
			* @warning This helper function is meant to be used only for Scripting Classes
			*/
			template<typename T>
			T* CreateInstance(const std::string& type)
			{
				//Check if the specific function is already loaded
				if (mCreateFunctions.find(type) == mCreateFunctions.end())
				{
					CreateFunc newFunc = nullptr;
					std::string funcName = "Create" + std::string(type);
					std::replace(funcName.begin(), funcName.end(), ' ', '_');
					newFunc = (CreateFunc)GetProcAddress((HMODULE)mHdll, funcName.c_str());
					if (!newFunc) return nullptr;
					mCreateFunctions.insert({ type, newFunc });
				}
				return static_cast<T*>(mCreateFunctions[type]());
			}

			/**
			* @brief Checks whether the specified entity has user defined component
			* @param type Type of user defined component to check as a String
			* @param entity Entity to check if it has user defined component
			* @return True if the entity has user defined component, false otherwise
			*/
			bool HasComponent(const std::string& type, Entity entity);

			/**
			* @brief Adds a user defined component to the specified entity
			* @param type Type of the user defined component as a String
			* @param entity Entity to which the user defined component
			* @return Pointer to the memory for the user defined component
			*/
			void* AddComponent(const std::string& type, Entity entity);

			/**
			* @brief Get a user defined component from the specified entity
			* @param type Type of the user defined component as a String
			* @param entity Entity to which the user defined component
			* @return Pointer to the memory for the user defined component
			*/
			void* GetComponent(const std::string& type, Entity entity);

			/**
			* @brief Removes a user defined component from the specified entity
			* @param type Type of the user defined component as a String
			* @param entity Entity to which the user defined component
			*/
			void RemoveComponent(const std::string& type, Entity entity);

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
			std::unordered_map<std::string, HasCompFunc> mHasComponentFunctions;
			std::unordered_map<std::string, EmplaceOrReplaceCompFunc> mEmplaceComponentFunctions;
			std::unordered_map<std::string, RemoveCompFunc> mRemoveComponentFunctions;
		};
	}
}