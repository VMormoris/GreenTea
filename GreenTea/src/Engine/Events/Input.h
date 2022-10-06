#pragma once

#include "Event.h"

namespace gte {

	/**
	* @brief Utility class for Quering if a particular key or mouse button is pressed
	*/
	class ENGINE_API Input {
	public:

		/**
		* @brief Query if a mouse button is pressed or not
		* @param type The specific button that the query is for
		* @returns True if the button is pressed, false otherwise
		*/
		static bool IsMouseButtonPressed(MouseButtonType type);

		/**
		* @brief Query if key is pressed or not
		* @param keycode Code for the key that the query is for
		* @returns True if the key is pressed, false otherwise
		*/
		static bool IsKeyPressed(KeyCode keycode);

	};

}