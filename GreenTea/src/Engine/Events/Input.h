#pragma once

#include "Event.h"

namespace gte {


	//Forward declaration(s)
	class Entity;
	class ScriptableEntity;

	/**
	* @brief Utility class for Quering if a particular key or mouse button is pressed
	*/
	class ENGINE_API Input {
	public:

		/**
		* @brief Query if a mouse button is pressed or not
		* @param type The specific button that the query is for
		* @return True if the button is pressed, false otherwise
		*/
		static bool IsMouseButtonPressed(MouseButtonType type);

		/**
		* @brief Query if key is pressed or not
		* @param keycode Code for the key that the query is for
		* @return True if the key is pressed, false otherwise
		*/
		static bool IsKeyPressed(KeyCode keycode);

		/**
		* @brief Get mouse position relative to the window
		* @param[out] x Mouse coordinate on the X axis
		* @param[out] y Mouse coordinate on the Y axis
		*/
		static void GetMousePos(uint32& x, uint32& y);

		/**
		* @brief Gets the entity that is currently hovered
		* @return An entity (might be invalid if no entity is hovered)
		*/
		static Entity GetHoveredEntity(void);

		/**
		* @brief Checks whether the specified Entity is hovered or not
		* @return True if it is hovered, false otherwise
		*/
		static bool IsHovered(Entity entity);

		/**
		* @brief Checks whether the specified Entity is hovered or not
		* @return True if it is hovered, false otherwise
		*/
		static bool IsHovered(ScriptableEntity* entity);

		/**
		* @brief Get the time in seconds since the game started
		* @return Seconds since start
		*/
		static float TimeSinceStart(void);
	};

}