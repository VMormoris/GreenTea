#include "DynamicLibLoader.h"

#include "GreenTeaWin.h"

namespace gte::internal {

	DynamicLibLoader::DynamicLibLoader(void)
	{
		mHdll = NULL;
		ZeroMemory(mLibsFilePath, 512);
	}

	void DynamicLibLoader::SetLibFile(const char* filepath)
	{
		size_t size = strlen(filepath);
		ENGINE_ASSERT(size < 512, "Filepath is too long!");
		memcpy(mLibsFilePath, filepath, size);
	}

	bool DynamicLibLoader::Load(const char* filepath)
	{
		SetLibFile(filepath);
		return Reload();
	}

	bool DynamicLibLoader::Reload(void)
	{
		Unload();
		mHdll = LoadLibraryA(mLibsFilePath);
		if (mHdll == NULL) { GTE_ERROR_LOG("Couldn't load library: ", mLibsFilePath); }
		return (mHdll != NULL);
	}

	void DynamicLibLoader::Unload(void)
	{
		if (mHdll != NULL)
		{
			bool result = FreeLibrary((HMODULE)mHdll);
			mCreateFunctions.clear();
			mHdll = NULL;
		}
	}
}