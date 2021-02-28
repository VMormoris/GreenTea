#include <GreenTea/Core/FileDialog.h>

#include <GreenTea/Core/Window.h>

#include <Windows.h>

namespace GTE {

	std::string CreateFileDialog(FileDialogType dialogType, const char* filters)
	{
		OPENFILENAMEA ofn;
		char szFile[256] = { 0 };
		memset(&ofn, 0, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = (HWND)Window::GetNativeWindow();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filters;
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

}