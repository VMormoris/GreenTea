#pragma once

#include "EngineCore.h"
#include <string>

namespace GTE {

	/**
	* @brief Enumaration for every kind of File Dialog
	*/
	enum class ENGINE_API FileDialogType : byte {
		None = 0x00,
		Open = 0x01,
		Save = 0x02
	};

	/**
	* @brief Creates a File Dialog Window
	* @param dialogType Type for the kind of FileDialog you are intresting for
	* @param filters Filters for what file you are intresting in
	* @returns A string containing the filepath of the choosen file
	* @warnings An empty string is returned if the Dialog window was canceled
	*/
	ENGINE_API std::string CreateFileDialog(FileDialogType dialogType, const char* filters);

}