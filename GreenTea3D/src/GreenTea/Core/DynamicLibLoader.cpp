#include "DynamicLibLoader.h"

namespace GTE {

	DynamicLibLoader::DynamicLibLoader(void)
	{
		m_hdll = NULL;
		ZeroMemory(m_LibsFilePath, 512);
	}

	bool DynamicLibLoader::Load(const char* filepath){
		size_t size = strlen(filepath);
		ENGINE_ASSERT(size < 512, "Filepath is too long!");
		memcpy(m_LibsFilePath, filepath, size);
		return Reload();
	}

	bool DynamicLibLoader::Reload(void)
	{
		Unload();
		m_hdll = LoadLibraryA(m_LibsFilePath);
		if (m_hdll == NULL) { GTE_ERROR_LOG("Couldn't load library: ", m_LibsFilePath); }
		return (m_hdll != NULL);
	}

	void DynamicLibLoader::Unload(void)
	{
		if (m_hdll != NULL)
		{
			FreeLibrary(m_hdll);
			m_CreateFunctions.clear();
			m_hdll = NULL;
		}
	}

}