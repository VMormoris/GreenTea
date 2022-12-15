#include "ProjectManager.h"

#include <Engine/Core/Context.h>

#include <windows.h>
#include <fstream>
#include <chrono>
#include <thread>

namespace gte::internal {

	bool ProjectManager::Reload(void)
	{
		mChanged.clear();

		bool changed = false;
		DWORD result = WaitForSingleObject(mOverlapped->hEvent, 0);
		if (result == WAIT_OBJECT_0)
		{
			DWORD bytes_transferred;
			GetOverlappedResult(mFileHandle, mOverlapped, &bytes_transferred, FALSE);

			FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)mChangeBuffer;
			uuid oldid;
			for (;;)
			{
				DWORD name_len = event->FileNameLength / sizeof(wchar_t);
				std::wstring ws(event->FileName, name_len);
#pragma warning(push)
#pragma warning(disable: 4244)
				std::string filepath = "Assets\\" + std::string(ws.begin(), ws.end());
#pragma warning(pop)
				if (!std::filesystem::is_directory(filepath))
				{
					switch (event->Action)
					{
					case FILE_ACTION_MODIFIED:
					{
						uuid id = ReloadFile(mProjectDir / (filepath));
						if (id.IsValid())
						{
							mChanged.push_back(id);
							changed = true;
							if (mFilePaths.find(id) == mFilePaths.end())//New file was added
								mFilePaths.insert({ id, filepath });
						}
						break;
					}
					case FILE_ACTION_REMOVED:
						Remove(filepath);
						changed = true;
						break;
					case FILE_ACTION_RENAMED_OLD_NAME:
					{
						oldid = Remove(filepath);
						changed = true;
						break;
					}
					case FILE_ACTION_RENAMED_NEW_NAME:
						mFilePaths.insert({ oldid, filepath });
						changed = true;
						break;
					default:
						break;
					}
				}
				// Are there more events to handle?
				if (event->NextEntryOffset)
					*((uint8_t**)&event) += event->NextEntryOffset;
				else
					break;
			}

			BOOL status = ReadDirectoryChangesW
			(
				mFileHandle,
				mChangeBuffer, 4096,
				TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
				NULL,
				mOverlapped, NULL
			);

			if (!status)
				exit(GetLastError());
		}
		

		auto prjname = std::filesystem::current_path().stem().string();
		auto srcDLL = mProjectDir / ("bin/Release-windows/" + prjname + "/" + prjname + ".dll");
		if (std::filesystem::last_write_time(".gt/Notifications") > mLastNotificationWrite)
		{
			mLastNotificationWrite = std::filesystem::last_write_time(".gt/Notifications");
			std::string notification;
			std::ifstream is(".gt/Notifications");
			std::getline(is, notification);
			is.close();
			if (notification.compare("BuildStarted") == 0)
			{
				GetContext()->ActiveScene->DestroyRuntime();
				GetContext()->DynamicLoader.Unload();
				mBuilding = true;
			}
			else if (notification.compare("BuildEnded") == 0)
			{
				auto dstDLL = ".gt/" + prjname + ".dll";
				std::filesystem::copy(srcDLL, dstDLL, std::filesystem::copy_options::overwrite_existing);
				GetContext()->ScriptEngine->Reload();
				GetContext()->DynamicLoader.Load(dstDLL.c_str());
				mBuilding = false;
			}
		}
		return changed;
	}

	void ProjectManager::LoadProject(const std::filesystem::path& projectdir)
	{
		if (!mOverlapped)
			mOverlapped = new OVERLAPPED;

		if (mFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(mOverlapped->hEvent);
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}

		FindFiles();

		mLastNotificationWrite = std::filesystem::last_write_time(".gt/Notifications");
		auto prjname = std::filesystem::current_path().stem().string();
		auto srcDLL ="bin/Release-windows/" + prjname + "/" + prjname + ".dll";
		auto dstDLL = ".gt/" + prjname + ".dll";
		if (std::filesystem::exists(srcDLL))
			std::filesystem::copy(srcDLL, dstDLL, std::filesystem::copy_options::overwrite_existing);
		if (!std::filesystem::exists(dstDLL))
		{
			GTE_WARN_LOG(prjname, ".dll wasn't found. Please build your project...");
			GetContext()->DynamicLoader.SetLibFile(dstDLL.c_str());
		}
		else
			GetContext()->DynamicLoader.Load(dstDLL.c_str());
		

		mFileHandle = CreateFileA
		(
			(projectdir/"Assets").string().c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (mFileHandle == INVALID_HANDLE_VALUE)
			exit(GetLastError());

		mOverlapped->hEvent = CreateEvent(NULL, FALSE, 0, NULL);

		BOOL status = ReadDirectoryChangesW
		(
			mFileHandle,
			mChangeBuffer, 4096,
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
			NULL,
			mOverlapped, NULL
		);

		if (!status)
			exit(GetLastError());
	}

	uuid ProjectManager::ReloadFile(const std::filesystem::path& filepath)
	{//Only reading header actually
		if (!std::filesystem::exists(filepath))
			return {};
		std::ifstream is(filepath.string(), std::ios::binary);
		std::string line;
		uint16 loaded = 0;
		uuid id;
		while (getline(is, line))
		{
			if (line[0] == '#')//Ignore as line comment
				continue;
			if (line.empty() || line[0] == '\n' || (line[0] == '\r' && line[1] == '\n'))//Second new line character marks the end of the header
				break;
			if (loaded == 1)
				id = line;
			loaded++;
		}
		is.close();
		ASSERT(id.IsValid(), "Asset: ",	filepath.stem().string() , " has not a valid UUID.");
		return id;
	}

	uuid ProjectManager::Remove(const std::string& filepath) noexcept
	{
		uuid id = {};
		for (const auto [key, val] : mFilePaths)
		{
			if (val.compare(filepath) == 0)
			{
				id = key;
				mFilePaths.erase(key);
				return id;
			}
		}
		return {};
	}

	[[nodiscard]] std::string ProjectManager::GetFilepath(const uuid& id) const noexcept
	{
		ASSERT((Exists(id) && id.IsValid()) || !id.IsValid(), "Specified ID doesn't exists");
		return id.IsValid() ? mFilePaths.at(id) : "";
	}

	[[nodiscard]] bool ProjectManager::Exists(const std::string& filepath) const noexcept
	{
		for (const auto& [key, file] : mFilePaths)
		{
			if (file.compare(filepath) == 0)
				return true;
		}
		return false;
	}

	[[nodiscard]] uuid ProjectManager::GetID(const std::string& filepath) const noexcept
	{
		for (const auto& [key, file] : mFilePaths)
		{
			if (file.compare(filepath) == 0)
				return key;
		}
		return {};
	}


	ProjectManager::ProjectManager(const std::filesystem::path& projectdir) noexcept { LoadProject(projectdir); }
	
	ProjectManager::~ProjectManager(void)
	{
		if (mFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(mOverlapped->hEvent);
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}

		if (mOverlapped)
			delete mOverlapped;
	}

	[[nodiscard]] bool ProjectManager::Exists(const uuid& id) const noexcept { return mFilePaths.find(id) != mFilePaths.end(); }
	[[nodiscard]] const std::vector<uuid>& ProjectManager::GetChanges(void) const noexcept { return mChanged; }

	void ProjectManager::FindFiles(void)
	{
		mFilePaths.clear();
		for (const auto entry : std::filesystem::recursive_directory_iterator(mProjectDir / "Assets"))
		{
			if (!entry.is_directory())
			{
				uuid id = ReloadFile(entry);
				mFilePaths.insert({ id, entry.path().string() });
			}
		}
	}

	[[nodiscard]] std::vector<uuid> ProjectManager::GetAssets(const std::initializer_list<std::string>& types) const noexcept
	{
		std::vector<uuid> ids;
		for (const auto& type: types)
		{
			for (const auto& [id, filepath] : mFilePaths)
			{
				const auto extension = filepath.substr(filepath.find_last_of('.'));
				if (type.compare(extension) == 0)
					ids.push_back(id);
			}
		}
		return ids;
	}

	[[nodiscard]] bool ProjectManager::IsBuilding(void) const noexcept { return mBuilding; }

}