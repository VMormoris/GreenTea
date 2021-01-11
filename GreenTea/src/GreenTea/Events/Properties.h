#ifndef _PROPERTIES
#define _PROPERTIES

#include "GreenTea/Core/EngineCore.h"

namespace GTE {

/**
* State of mouse's button or keyboard's key is: was pressed
*/
#define PRESSED 0
/**
* State of mouse's button or keyboard's key is: was released
*/
#define RELEASED 1

	/**
	* Struct for Representing the properties of a Window Event
	*/
	struct ENGINE_API WindowProp {
		/**
		* @brief Window's width
		*/
		uint32 width ;
		/**
		* @brief Window's height
		*/
		uint32 height;
		/**
		* @brief Window's coordinate on the X - axis
		*/
		int x;
		/**
		* @brief Window's coordinate on the Y - axis
		*/
		int y;

		WindowProp(void) {
			x = y = -1;
			width = height = 0;
		}
	};

	/**
	* Enumeration for the mouse's button that is was pressed or released
	*/
	enum class ENGINE_API MouseButtonType : byte {
		/**
		* All mouse's buttons that are not supported
		*/
		None=0x00,
		/**
		* Left click
		*/
		Left = 0x01,
		/**
		* Wheel was pressed
		*/
		Wheel = 0x02,
		/**
		* Right click
		*/
		Right = 0x03
	};

	/**
	* Struct for Representing the properties of mouse's button 
	*/
	struct ENGINE_API MouseButtonProp {
		/**
		* Type of Mouse button (aka which button)
		*/
		MouseButtonType type;
		/**
		* Button's state (Pressed or Released)
		*/
		char state;
		/**
		* Default Constructor for initialization of the fields
		*/
		MouseButtonProp(void) {
			type = MouseButtonType::None;
			state = -1;
		}
	};

	/**
	* Struct for Representing the properties of a Window Event
	*/
	struct ENGINE_API MouseProp {
		/**
		* Properties for mouse button
		*/
		MouseButtonProp button;
		/**
		* Mouse's X coordinate
		*/
		int x;
		/**
		* Mouse's Y coordinate
		*/
		int y;
		/**
		* Wheel's "force" on X axis
		*/
		int wheel_dx;
		/**
		* Wheel's "force" on Y axis
		*/
		int wheel_dy;
	};

	/**
	* @brief Enumaration for all avaliable Keys that Engine supports
	*/
	enum class KeyCode : char {
		///Invalid Key (0xFF)
		INVALID = -1,//0xFF produces warning :(

		//Numbers

		D0 = 0x00,
		D1 = 0x01,
		D2 = 0x02,
		D3 = 0x03,
		D4 = 0x04,
		D5 = 0x05,
		D6 = 0x06,
		D7 = 0x07,
		D8 = 0x08,
		D9 = 0x09,

		//Numpads Keys

		NUMPAD0 = 0x0A,
		NUMPAD1 = 0x0B,
		NUMPAD2 = 0x0C,
		NUMPAD3 = 0x0D,
		NUMPAD4 = 0x0E,
		NUMPAD5 = 0x0F,
		NUMPAD6 = 0x10,
		NUMPAD7 = 0x11,
		NUMPAD8 = 0x12,
		NUMPAD9 = 0x13,

		//Function keys

		F1 = 0x14,
		F2 = 0x15,
		F3 = 0x16,
		F4 = 0x17,
		F5 = 0x18,
		F6 = 0x19,
		F7 = 0x1A,
		F8 = 0x1B,
		F9 = 0x1C,
		F10 = 0x1D,
		F11 = 0x1E,
		F12 = 0x1F,
		
		//Other keys
		
		ESCAPE = 0x20,
		BACKQUOTE = 0x21,
		TAB = 0x22,
		CAPSLOCK = 0x23,
		LEFT_SHIFT = 0x24,
		LEFT_CONTROL = 0x25,
		LEFT_ALT = 0x26,
		RIGHT_SHIFT = 0x27,
		RIGHT_CONTROL = 0x28,
		WINDOWS = 0x29,
		ENTER = 0x2A,
		BACKSPACE = 0x2B,
		SPACE = 0x2C,
		MINUS = 0x2D,
		PLUS = 0x2E,

		//Ascii character keys

		A = 0x30,
		B = 0x31,
		C = 0x32,
		D = 0x33,
		E = 0x34,
		F = 0x35,
		G = 0x36,
		H = 0x37,
		I = 0x38,
		J = 0x39,
		K = 0x3A,
		L = 0x3B,
		M = 0x3C,
		N = 0x3D,
		O = 0x3E,
		P = 0x3F,
		Q = 0x40,
		R = 0x41,
		S = 0x42,
		T = 0x43,
		U = 0x44,
		V = 0x45,
		W = 0x46,
		X = 0x47,
		Y = 0x48,
		Z = 0x49,

		//Other keys
		
		OPEN_BRACKET = 0x4A,
		CLOSE_BRACKET = 0x4B,
		SLASH = 0x4C,
		BACKSLASH = 0x4D,
		QUOTE = 0x4E,
		SEMICOLON = 0x4F,
		COMMA = 0x50,
		FULLSTOP =  0x51,

		//Arrow keys
		
		UP = 0x52,
		DOWN = 0x53,
		LEFT = 0x54,
		RIGHT = 0x55

	};
	/**
	* Struct for Representing the properties of keyboard's keys
	*/
	struct ENGINE_API KeyboardProp {
		
		/**
		* Code for representing which of the keyboard's keys is being pressed or released
		*/
		KeyCode code;
		/**
		* Key's state (Pressed or Released)
		*/
		char state;

		KeyboardProp(void) {
			code = KeyCode::INVALID;
			state = -1;
		}
	};

}

//Macros for number keys
#define KEY_0 ::GTE::KeyCode::D0
#define KEY_1 ::GTE::KeyCode::D1
#define KEY_2 ::GTE::KeyCode::D2
#define KEY_3 ::GTE::KeyCode::D3
#define KEY_4 ::GTE::KeyCode::D4
#define KEY_5 ::GTE::KeyCode::D5
#define KEY_6 ::GTE::KeyCode::D6
#define KEY_7 ::GTE::KeyCode::D7
#define KEY_8 ::GTE::KeyCode::D8
#define KEY_9 ::GTE::KeyCode::D9

//Macros for numpad's key
#define KEY_NUMPAD0 ::GTE::KeyCode::NUMPAD0
#define KEY_NUMPAD1 ::GTE::KeyCode::NUMPAD1
#define KEY_NUMPAD2 ::GTE::KeyCode::NUMPAD2
#define KEY_NUMPAD3 ::GTE::KeyCode::NUMPAD3
#define KEY_NUMPAD4 ::GTE::KeyCode::NUMPAD4
#define KEY_NUMPAD5 ::GTE::KeyCode::NUMPAD5
#define KEY_NUMPAD6 ::GTE::KeyCode::NUMPAD6
#define KEY_NUMPAD7 ::GTE::KeyCode::NUMPAD7
#define KEY_NUMPAD8 ::GTE::KeyCode::NUMPAD8
#define KEY_NUMPAD9 ::GTE::KeyCode::NUMPAD9

//Macros for function keys
#define KEY_F1 ::GTE::KeyCode::F1
#define KEY_F2 ::GTE::KeyCode::F2
#define KEY_F3 ::GTE::KeyCode::F3
#define KEY_F4 ::GTE::KeyCode::F4
#define KEY_F5 ::GTE::KeyCode::F5
#define KEY_F6 ::GTE::KeyCode::F6
#define KEY_F7 ::GTE::KeyCode::F7
#define KEY_F8 ::GTE::KeyCode::F8
#define KEY_F9 ::GTE::KeyCode::F9
#define KEY_F10 ::GTE::KeyCode::F10
#define KEY_F11 ::GTE::KeyCode::F11
#define KEY_F12 ::GTE::KeyCode::F12

//Macros for other keys
#define KEY_ESCAPE ::GTE::KeyCode::ESCAPE
#define KEY_BACKQUOTE ::GTE::KeyCode::BACKQUOTE
#define KEY_TAB ::GTE::KeyCode::TAB
#define KEY_CAPSLOCK ::GTE::KeyCode::CAPSLOCK
#define KEY_LEFT_SHIFT ::GTE::KeyCode::LEFT_SHIFT
#define KEY_LEFT_CONTROL ::GTE::KeyCode::LEFT_CONTROL
#define KEY_LEFT_ALT ::GTE::KeyCode::LEFT_ALT
#define KEY_RIGHT_SHIFT ::GTE::KeyCode::RIGHT_SHIFT
#define KEY_RIGHT_CONTROL ::GTE::KeyCode::RIGHT_CONTROL
#define KEY_WINDOWS ::GTE::KeyCode::WINDOWS
#define KEY_ENTER ::GTE::KeyCode::ENTER
#define KEY_BACKSPACE ::GTE::KeyCode::BACKSPACE
#define KEY_SPACE ::GTE::KeyCode::SPACE
#define KEY_MINUS ::GTE::KeyCode::MINUS
#define KEY_PLUS ::GTE::KeyCode::PLUS

//Macros for character keys
#define KEY_A ::GTE::KeyCode::A
#define KEY_B ::GTE::KeyCode::B
#define KEY_C ::GTE::KeyCode::C
#define KEY_D ::GTE::KeyCode::D
#define KEY_E ::GTE::KeyCode::E
#define KEY_F ::GTE::KeyCode::F
#define KEY_G ::GTE::KeyCode::G
#define KEY_H ::GTE::KeyCode::H
#define KEY_I ::GTE::KeyCode::I
#define KEY_J ::GTE::KeyCode::J
#define KEY_K ::GTE::KeyCode::K
#define KEY_L ::GTE::KeyCode::L
#define KEY_M ::GTE::KeyCode::M
#define KEY_N ::GTE::KeyCode::N
#define KEY_O ::GTE::KeyCode::O
#define KEY_P ::GTE::KeyCode::P
#define KEY_Q ::GTE::KeyCode::Q
#define KEY_R ::GTE::KeyCode::R
#define KEY_S ::GTE::KeyCode::S
#define KEY_T ::GTE::KeyCode::T
#define KEY_U ::GTE::KeyCode::U
#define KEY_V ::GTE::KeyCode::V
#define KEY_W ::GTE::KeyCode::W
#define KEY_X ::GTE::KeyCode::X
#define KEY_Y ::GTE::KeyCode::Y
#define KEY_Z ::GTE::KeyCode::Z

//Macros for other keys
#define KEY_OPEN_BRACKET ::GTE::KeyCode::OPEN_BRACKET
#define KEY_CLOSE_BRACKET ::GTE::KeyCode::CLOSE_BRACKET
#define KEY_SLASH ::GTE::KeyCode::SLASH
#define KEY_BACKSLASH ::GTE::KeyCode::BACKSLASH
#define KEY_QUOTE ::GTE::KeyCode::QUOTE
#define KEY_SEMICOLON ::GTE::KeyCode::SEMICOLON
#define KEY_COMMA ::GTE::KeyCode::COMMA
#define KEY_FULLSTOP ::GTE::KeyCode::FULLSTOP

//Macros for Arrow keys
#define KEY_UP ::GTE::KeyCode::UP
#define KEY_DOWN ::GTE::KeyCode::DOWN
#define KEY_LEFT ::GTE::KeyCode::LEFT
#define KEY_RIGHT ::GTE::KeyCode::RIGHT

#endif 
