#include "DynamicLibLoader.h"

#include "GreenTeaWin.h"
#include "Engine/Scene/Entity.h"

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
		mCreateFunctions.clear();
		mHasComponentFunctions.clear();
		mEmplaceComponentFunctions.clear();
		mRemoveComponentFunctions.clear();

		if (mHdll != NULL)
		{
			bool result = FreeLibrary((HMODULE)mHdll);
			mCreateFunctions.clear();
			mHdll = NULL;
		}
	}
	
	bool DynamicLibLoader::HasComponent(const std::string& type, Entity entity)
	{
		//Check if the specific function is already loaded
		if (mHasComponentFunctions.find(type) == mHasComponentFunctions.end())
		{
			HasCompFunc newFunc = nullptr;
			std::string funcName = "Has" + std::string(type);
			std::replace(funcName.begin(), funcName.end(), ' ', '_');
			newFunc = (HasCompFunc)GetProcAddress((HMODULE)mHdll, funcName.c_str());
			if (!newFunc) return false;
			mHasComponentFunctions.insert({ type, newFunc });
		}
		return mHasComponentFunctions[type](entity);
	}

	void* DynamicLibLoader::AddComponent(const std::string& type, Entity entity)
	{
		std::string funcName = "Create" + std::string(type);
		std::replace(funcName.begin(), funcName.end(), ' ', '_');
		//Check if the specific function is already loaded
		if (mEmplaceComponentFunctions.find(funcName) == mEmplaceComponentFunctions.end())
		{
			EmplaceOrReplaceCompFunc newFunc = nullptr;
			newFunc = (EmplaceOrReplaceCompFunc)GetProcAddress((HMODULE)mHdll, funcName.c_str());
			if (!newFunc) return nullptr;
			mEmplaceComponentFunctions.insert({ funcName, newFunc });
		}
		return mEmplaceComponentFunctions[funcName](entity);
	}
	
	void* DynamicLibLoader::GetComponent(const std::string& type, Entity entity)
	{
		std::string funcName = "Get" + std::string(type);
		std::replace(funcName.begin(), funcName.end(), ' ', '_');
		//Check if the specific function is already loaded
		if (mEmplaceComponentFunctions.find(funcName) == mEmplaceComponentFunctions.end())
		{
			EmplaceOrReplaceCompFunc newFunc = nullptr;
			newFunc = (EmplaceOrReplaceCompFunc)GetProcAddress((HMODULE)mHdll, funcName.c_str());
			if (!newFunc) return nullptr;
			mEmplaceComponentFunctions.insert({ funcName, newFunc });
		}
		return mEmplaceComponentFunctions[funcName](entity);
	}
	
	void DynamicLibLoader::RemoveComponent(const std::string& type, Entity entity)
	{
		//Check if the specific function is already loaded
		if (mRemoveComponentFunctions.find(type) == mRemoveComponentFunctions.end())
		{
			RemoveCompFunc newFunc = nullptr;
			std::string funcName = "Remove" + std::string(type);
			std::replace(funcName.begin(), funcName.end(), ' ', '_');
			newFunc = (RemoveCompFunc)GetProcAddress((HMODULE)mHdll, funcName.c_str());
			if (!newFunc) return;
			mRemoveComponentFunctions.insert({ type, newFunc });
		}
		return mRemoveComponentFunctions[type](entity);
	}
}