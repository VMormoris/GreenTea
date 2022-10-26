#pragma once
#include <Engine/Core/Engine.h>

namespace gte {

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
		/**
		* @brief Click event
		*/
		Click = 0x0800,
		/**
		* @brief Starting hovering object
		*/
		HoverEnter = 0x1000,
		/**
		* @brief Stopped hovering object
		*/
		HoverExit = 0x2000
	};

	enum class ENGINE_API MouseButtonType : byte {
		/**
		* Left click
		*/
		Left = 0x00,
		/**
		* Right click
		*/
		Right = 0x01,
		/**
		* Wheel was pressed
		*/
		Wheel = 0x02
	};

	enum class ENGINE_API KeyCode : uint16
	{
		// From glfw3.h
		SPACE = 32,
		Apostrophe = 39, /* ' */
		COMMA = 44, /* , */
		MINUS = 45, /* - */
		PERIOD = 46, /* . */
		SLASH = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		SEMICOLON = 59, /* ; */
		EQUAL = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LEFT_BRACKET = 91,  /* [ */
		BACKSLASH = 92,  /* \ */
		RIGHT_BRACKET = 93,  /* ] */
		BACKQUOTE = 96,  /* ` */

		WORLD1 = 161, /* non-US #1 */
		WORLD2 = 162, /* non-US #2 */

		/* Function keys */
		ESCAPE = 256,
		ENTER = 257,
		TAB = 258,
		BACKSPACE = 259,
		INSERT = 260,
		DEL = 261,
		RIGHT = 262,
		LEFT = 263,
		DOWN = 264,
		UP = 265,
		PAGE_UP = 266,
		PAGE_DOWN = 267,
		HOME = 268,
		END = 269,
		CAPSLOCK = 280,
		SCROLL_LOCK = 281,
		NUM_LOCK = 282,
		PRINT_SCREEN = 283,
		PAUSE = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		NUMPAD0 = 320,
		NUMPAD1 = 321,
		NUMPAD2 = 322,
		NUMPAD3 = 323,
		NUMPAD4 = 324,
		NUMPAD5 = 325,
		NUMPAD6 = 326,
		NUMPAD7 = 327,
		NUMPAD8 = 328,
		NUMPAD9 = 329,
		NUMPAD_DECIMAL = 330,
		NUMPAD_DIVIDE = 331,
		NUMPAD_MULTIPLY = 332,
		NUMPAD_SUBTRACT = 333,
		NUMPAD_ADD = 334,
		NUMPAD_ENTER = 335,
		NUMPAD_EQUAL = 336,

		LEFT_SHIFT = 340,
		LEFT_CONTROL = 341,
		LEFT_ALT = 342,
		LEFT_SUPPER = 343,
		RIGHT_SHIFT = 344,
		RIGHT_CONTROL = 345,
		RIGHT_ALT = 346,
		RIGHT_SUPER = 347,
		MENU = 348
	};

}

//Macros for number keys
#define KEY_0 ::gte::KeyCode::D0
#define KEY_1 ::gte::KeyCode::D1
#define KEY_2 ::gte::KeyCode::D2
#define KEY_3 ::gte::KeyCode::D3
#define KEY_4 ::gte::KeyCode::D4
#define KEY_5 ::gte::KeyCode::D5
#define KEY_6 ::gte::KeyCode::D6
#define KEY_7 ::gte::KeyCode::D7
#define KEY_8 ::gte::KeyCode::D8
#define KEY_9 ::gte::KeyCode::D9

//Macros for numpad's key
#define KEY_NUMPAD0 ::gte::KeyCode::NUMPAD0
#define KEY_NUMPAD1 ::gte::KeyCode::NUMPAD1
#define KEY_NUMPAD2 ::gte::KeyCode::NUMPAD2
#define KEY_NUMPAD3 ::gte::KeyCode::NUMPAD3
#define KEY_NUMPAD4 ::gte::KeyCode::NUMPAD4
#define KEY_NUMPAD5 ::gte::KeyCode::NUMPAD5
#define KEY_NUMPAD6 ::gte::KeyCode::NUMPAD6
#define KEY_NUMPAD7 ::gte::KeyCode::NUMPAD7
#define KEY_NUMPAD8 ::gte::KeyCode::NUMPAD8
#define KEY_NUMPAD9 ::gte::KeyCode::NUMPAD9

//Macros for function keys
#define KEY_F1 ::gte::KeyCode::F1
#define KEY_F2 ::gte::KeyCode::F2
#define KEY_F3 ::gte::KeyCode::F3
#define KEY_F4 ::gte::KeyCode::F4
#define KEY_F5 ::gte::KeyCode::F5
#define KEY_F6 ::gte::KeyCode::F6
#define KEY_F7 ::gte::KeyCode::F7
#define KEY_F8 ::gte::KeyCode::F8
#define KEY_F9 ::gte::KeyCode::F9
#define KEY_F10 ::gte::KeyCode::F10
#define KEY_F11 ::gte::KeyCode::F11
#define KEY_F12 ::gte::KeyCode::F12

//Macros for other keys
#define KEY_ESCAPE ::gte::KeyCode::ESCAPE
#define KEY_BACKQUOTE ::gte::KeyCode::BACKQUOTE
#define KEY_TAB ::gte::KeyCode::TAB
#define KEY_CAPSLOCK ::gte::KeyCode::CAPSLOCK
#define KEY_LEFT_SHIFT ::gte::KeyCode::LEFT_SHIFT
#define KEY_LEFT_CONTROL ::gte::KeyCode::LEFT_CONTROL
#define KEY_LEFT_ALT ::gte::KeyCode::LEFT_ALT
#define KEY_RIGHT_SHIFT ::gte::KeyCode::RIGHT_SHIFT
#define KEY_RIGHT_CONTROL ::gte::KeyCode::RIGHT_CONTROL
#define KEY_WINDOWS ::gte::KeyCode::MENU
#define KEY_ENTER ::gte::KeyCode::ENTER
#define KEY_BACKSPACE ::gte::KeyCode::BACKSPACE
#define KEY_SPACE ::gte::KeyCode::SPACE
#define KEY_MINUS ::gte::KeyCode::MINUS
#define KEY_PLUS ::gte::KeyCode::EQUAL

//Macros for character keys
#define KEY_A ::gte::KeyCode::A
#define KEY_B ::gte::KeyCode::B
#define KEY_C ::gte::KeyCode::C
#define KEY_D ::gte::KeyCode::D
#define KEY_E ::gte::KeyCode::E
#define KEY_F ::gte::KeyCode::F
#define KEY_G ::gte::KeyCode::G
#define KEY_H ::gte::KeyCode::H
#define KEY_I ::gte::KeyCode::I
#define KEY_J ::gte::KeyCode::J
#define KEY_K ::gte::KeyCode::K
#define KEY_L ::gte::KeyCode::L
#define KEY_M ::gte::KeyCode::M
#define KEY_N ::gte::KeyCode::N
#define KEY_O ::gte::KeyCode::O
#define KEY_P ::gte::KeyCode::P
#define KEY_Q ::gte::KeyCode::Q
#define KEY_R ::gte::KeyCode::R
#define KEY_S ::gte::KeyCode::S
#define KEY_T ::gte::KeyCode::T
#define KEY_U ::gte::KeyCode::U
#define KEY_V ::gte::KeyCode::V
#define KEY_W ::gte::KeyCode::W
#define KEY_X ::gte::KeyCode::X
#define KEY_Y ::gte::KeyCode::Y
#define KEY_Z ::gte::KeyCode::Z

//Macros for other keys
#define KEY_OPEN_BRACKET ::gte::KeyCode::LEFT_BRACKET
#define KEY_CLOSE_BRACKET ::gte::KeyCode::RIGHT_BRACKET
#define KEY_SLASH ::gte::KeyCode::SLASH
#define KEY_BACKSLASH ::gte::KeyCode::BACKSLASH
#define KEY_QUOTE ::gte::KeyCode::COMMA
#define KEY_SEMICOLON ::gte::KeyCode::SEMICOLON
#define KEY_COMMA ::gte::KeyCode::COMMA
#define KEY_FULLSTOP ::gte::KeyCode::PERIOD

//Macros for Arrow keys
#define KEY_UP ::gte::KeyCode::UP
#define KEY_DOWN ::gte::KeyCode::DOWN
#define KEY_LEFT ::gte::KeyCode::LEFT
#define KEY_RIGHT ::gte::KeyCode::RIGHT