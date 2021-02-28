#ifndef _TYPES
#define _TYPES

#include "GreenTea/Core/EngineCore.h"

namespace GTE {

	/**
	* @brief Enumeration for Event's type
	*/
	enum class ENGINE_API EventType : int16 {
		/**
		* @brief All Event types that are not supported
		*/
		None = 0x0000,
		/**
		* @brief Window Close was triggered
		*/
		WindowClose = 0x0001,
		/**
		* @brief Window being resized
		*/
		WindowResize = 0x0002,
		/**
		* @brief Window being moved
		*/
		WindowMove = 0x0004,
		/**
		* @brief Window got on focus
		*/
		WindowGainFocus = 0x0008,
		/**
		* @brief Window got out of focus
		*/
		WindowLostFocus = 0x0010,
		/**
		* @brief Keyboard's key is being pressed
		*/
		KeyPressed = 0x0020,
		/**
		* @brief Keyboard's key was released
		*/
		KeyReleased = 0x0040,
		/**
		* @brief Mouse's button is being pressed
		*/
		MouseButtonPressed = 0x0080,
		/**
		* @brief Mouse's button was released
		*/
		MouseButtonReleased = 0x0100,
		/**
		* @brief Mouse is moving
		*/
		MouseMove = 0x0200,
		/**
		* @brief Scroll wheel is being moved
		*/
		MouseScroll = 0x0400,
	};

	ENGINE_API EventType operator|(EventType left, EventType right);
	ENGINE_API EventType operator&(EventType left, EventType right);

	/**
	* @brief Enumeration for different Events that belong to the same category
	*/
	enum class ENGINE_API EventCategory : byte {
		/**
		* @brief All categories that are not supported
		*/
		None = 0x00,
		/**
		* @brief All supported window events
		*/
		WindowEvent = 0x01,
		/**
		* @brief All supported keyboard events
		*/
		KeyboardEvent = 0x02,
		/**
		* @brief All supported mouse events
		*/
		MouseEvent = 0x03,
	};

}

#endif
