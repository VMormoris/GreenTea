#include <Engine/Core/FileDialog.h>
#include <Engine/Core/Context.h>

#include <filesystem>

#include <Windows.h>
#include <shlobj_core.h>

namespace gte::internal {
	
	std::string CreateFileDialog(FileDialogType dialogType, const char* filters)
	{
		const auto dir = (std::filesystem::current_path()/"Assets").string();
		OPENFILENAMEA ofn;
		char szFile[256] = { 0 };
		memset(&ofn, 0, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		Window* window = GetContext()->GlobalWindow;
		ofn.hwndOwner = (HWND)window->GetNativeWindow();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filters;
		ofn.lpstrInitialDir = dir.c_str();
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		switch (dialogType)
		{
		case FileDialogType::Open:
			if (GetOpenFileNameA(&ofn) == TRUE)
				return ofn.lpstrFile;
			break;
		case FileDialogType::Save:
			if (GetSaveFileNameA(&ofn) == TRUE)
				return ofn.lpstrFile;
			break;
		default:
			break;
		}

		return std::string();
	}

	std::string PeekDirectory(void)
	{
		BROWSEINFOA odn;
		char szFile[256] = { 0 };
		memset(&odn, 0, sizeof(BROWSEINFOA));
		Window* window = GetContext()->GlobalWindow;
		odn.hwndOwner = (HWND)window->GetNativeWindow();

		ITEMIDLIST* pidl = static_cast<ITEMIDLIST*>(SHBrowseForFolderA(&odn));
		if (pidl)
		{
			char folder[_MAX_PATH];
			SHGetPathFromIDListA(pidl, folder);
			SHFree(pidl);
			return std::string(folder);
		}
		return std::string();
	}

}