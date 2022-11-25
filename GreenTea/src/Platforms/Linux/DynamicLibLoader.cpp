#include <Engine/Core/DynamicLibLoader.h>

#include <cstring>
#include <dlfcn.h>

namespace gte::internal {

	DynamicLibLoader::DynamicLibLoader(void)
	{
		mHdll = NULL;
		memset(mLibsFilePath, 0, 512);
	}

	bool DynamicLibLoader::Load(const char* filepath) {
		size_t size = strlen(filepath);
		ENGINE_ASSERT(size < 512, "Filepath is too long!");
		memcpy(mLibsFilePath, filepath, size);
		return Reload();
	}

	bool DynamicLibLoader::Reload(void)
	{
		Unload();
		mHdll = dlopen(mLibsFilePath, RTLD_NOW);
		if (mHdll == NULL) { GTE_ERROR_LOG("Couldn't load library: ", mLibsFilePath); }
		return (mHdll != NULL);
	}

	void DynamicLibLoader::Unload(void)
	{
		if (mHdll != NULL)
		{
			bool result = dlclose(mHdll);
			mCreateFunctions.clear();
			mHdll = NULL;
		}
	}
}