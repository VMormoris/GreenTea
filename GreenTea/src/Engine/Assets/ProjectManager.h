#pragma once

#include <Engine/Core/uuid.h>

#include <filesystem>
#include <unordered_map>

//Forward declaration(s)
struct _OVERLAPPED;

namespace gte::internal {

	/**
	* @brief Monitors projects for File changes (Only assets for now)
	*/
	class ENGINE_API ProjectManager {
	public:
		//Constructor(s) & Destructor
		ProjectManager(void) = default;
		ProjectManager(const std::filesystem::path& projectdir) noexcept;
		~ProjectManager(void) noexcept;

		/**
		* @brief Checks whether the specified id exists or not
		* @param id UUID for the Asset to be checked
		*/
		[[nodiscard]] bool Exists(const uuid& id) const noexcept;
		
		/**
		* @brief Checks whether the specified id exists or not
		* @param filepath String for filepath where the Asset to be checked is located
		*/
		[[nodiscard]] bool Exists(const std::string& filepath) const noexcept;

		/**
		* @brief Changes the directory the project manager checks
		* @param projectdir Base directory of a GreenTea project
		*/
		void LoadProject(const std::filesystem::path& projectdir);
		
		/**
		* @brief Returns the filepath for the specified identiefier
		* @param id Identifier for the Asset you are looking for
		* @return A string containing the absolute path of the Asset or an empty string
		*/
		[[nodiscard]] std::string GetFilepath(const uuid& id) const;

		[[nodiscard]] uuid GetID(const std::string& filepath) const noexcept;

		[[nodiscard]] std::vector<uuid> GetAssets(const std::initializer_list<std::string>& types) const noexcept;

		void FindFiles(void);

#ifndef GT_DIST
		/**
		* @brief Checks for changes in assets for the project
		* @return Whether was a change or not
		*/
		[[nodiscard]] bool Reload(void);

		/**
		* @brief Returns the identifiers of the assets that may need to be reloaded since the last time Reload called
		*/
		[[nodiscard]] const std::vector<uuid>& GetChanges(void) const noexcept;
		[[nodiscard]] bool IsBuilding(void) const noexcept;
#endif

	private:

		uuid ReloadFile(const std::filesystem::path& filepath);

#ifndef GT_DIST
		uuid Remove(const std::string& filepath) noexcept;
#endif

	private:
		std::unordered_map<uuid, std::string> mFilePaths;
		std::filesystem::path mProjectDir;
#ifndef GT_DIST
		void* mFileHandle = (void*)(int64)-1;
		_OVERLAPPED* mOverlapped = nullptr;
		byte mChangeBuffer[4096] = { 0 };
		std::vector<uuid> mChanged;
		std::filesystem::file_time_type mLastNotificationWrite;
		bool mBuilding = false;
#endif
	};

}