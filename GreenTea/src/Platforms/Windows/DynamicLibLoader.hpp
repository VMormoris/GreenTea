#pragma once

#include <Engine/Core/DynamicLibLoader.h>

namespace gte::internal{

	template<typename T>
	inline T* DynamicLibLoader::CreateInstance(const std::string& type)
	{
		//Check if the specific function is already loaded
		if (mCreateFunctions.find(type) == mCreateFunctions.end())
		{
			CreateFunc newFunc = NULL;
			std::string funcName = "Create" + std::string(type);
			std::replace(funcName.begin(), funcName.end(), ' ', '_');
			newFunc = (CreateFunc)GetProcAddress((HMODULE)mHdll, funcName.c_str());
			if (!newFunc) return nullptr;
			mCreateFunctions.insert({ type, newFunc });
		}
		return static_cast<T*>(mCreateFunctions[type]());
	}

}