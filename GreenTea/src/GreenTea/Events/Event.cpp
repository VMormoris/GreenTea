#include <SDL.h>
#include "Event.h"
#include "GreenTea/Core/Logger.h"
#include <unordered_map>

#include <imgui.h>
#include <imgui_impl_sdl.h>

static std::unordered_map<int32_t, GTE::KeyCode> keymap =
{
	{SDLK_0, KEY_0},
	{SDLK_1, KEY_1},
	{SDLK_2, KEY_2},
	{SDLK_3, KEY_3},
	{SDLK_4, KEY_4},
	{SDLK_5, KEY_5},
	{SDLK_6, KEY_6},
	{SDLK_7, KEY_7},
	{SDLK_8, KEY_8},
	{SDLK_9, KEY_9},
	{SDLK_KP_0, KEY_NUMPAD0},
	{SDLK_KP_1, KEY_NUMPAD1},
	{SDLK_KP_2, KEY_NUMPAD2},
	{SDLK_KP_3, KEY_NUMPAD3},
	{SDLK_KP_4, KEY_NUMPAD4},
	{SDLK_KP_5, KEY_NUMPAD5},
	{SDLK_KP_6, KEY_NUMPAD6},
	{SDLK_KP_7, KEY_NUMPAD7},
	{SDLK_KP_8, KEY_NUMPAD8},
	{SDLK_KP_9, KEY_NUMPAD9},
	{SDLK_F1, KEY_F1},
	{SDLK_F2, KEY_F2},
	{SDLK_F3, KEY_F3},
	{SDLK_F4, KEY_F4},
	{SDLK_F5, KEY_F5},
	{SDLK_F6, KEY_F6},
	{SDLK_F7, KEY_F7},
	{SDLK_F8, KEY_F8},
	{SDLK_F9, KEY_F9},
	{SDLK_F10, KEY_F10},
	{SDLK_F11, KEY_F11},
	{SDLK_F12, KEY_F12},
	{SDLK_ESCAPE, KEY_ESCAPE},
	{SDLK_BACKQUOTE, KEY_BACKQUOTE},
	{SDLK_TAB, KEY_TAB},
	{SDLK_CAPSLOCK, KEY_CAPSLOCK},
	{SDLK_LSHIFT, KEY_LEFT_SHIFT},
	{SDLK_RSHIFT, KEY_RIGHT_SHIFT},
	{SDLK_LCTRL, KEY_LEFT_CONTROL},
	{SDLK_RCTRL, KEY_RIGHT_CONTROL},
	{SDLK_LALT, KEY_LEFT_ALT},
	{SDLK_APPLICATION, KEY_WINDOWS},
	{SDLK_RETURN, KEY_ENTER},
	{SDLK_BACKSPACE, KEY_BACKSPACE},
	{SDLK_SPACE, KEY_SPACE},
	{SDLK_MINUS, KEY_MINUS},
	{SDLK_SPACE, KEY_SPACE},
	{SDLK_PLUS, KEY_PLUS},
	{SDLK_a, KEY_A},
	{SDLK_b, KEY_B},
	{SDLK_c, KEY_C},
	{SDLK_d, KEY_D},
	{SDLK_e, KEY_E},
	{SDLK_f, KEY_F},
	{SDLK_g, KEY_G},
	{SDLK_h, KEY_H},
	{SDLK_i, KEY_I},
	{SDLK_j, KEY_J},
	{SDLK_k, KEY_K},
	{SDLK_l, KEY_L},
	{SDLK_m, KEY_M},
	{SDLK_n, KEY_N},
	{SDLK_o, KEY_O},
	{SDLK_p, KEY_P},
	{SDLK_q, KEY_Q},
	{SDLK_r, KEY_R},
	{SDLK_s, KEY_S},
	{SDLK_t, KEY_T},
	{SDLK_u, KEY_U},
	{SDLK_v, KEY_V},
	{SDLK_w, KEY_W},
	{SDLK_x, KEY_X},
	{SDLK_y, KEY_Y},
	{SDLK_z, KEY_Z},
	{SDLK_LEFTBRACKET, KEY_OPEN_BRACKET},
	{SDLK_RIGHTBRACKET, KEY_CLOSE_BRACKET},
	{SDLK_SLASH, KEY_SLASH},
	{SDLK_BACKSLASH, KEY_BACKSLASH},
	{SDLK_QUOTE, KEY_QUOTE},
	{SDLK_SEMICOLON, KEY_SEMICOLON},
	{SDLK_COMMA, KEY_COMMA},
	{SDLK_PERIOD, KEY_FULLSTOP},
	{SDLK_UP, KEY_UP},
	{SDLK_DOWN, KEY_DOWN},
	{SDLK_RIGHT, KEY_RIGHT},
	{SDLK_LEFT, KEY_LEFT}
};

namespace GTE {


	bool PollEvent(Event* event)
	{ 
		SDL_Event sdl_event;
		if(!SDL_PollEvent(&sdl_event)) return false;

		//Send events to ImGui
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplSDL2_ProcessEvent(&sdl_event);

		switch (sdl_event.type) {
		case SDL_QUIT:
			event->category = EventCategory::WindowEvent;
			event->type = EventType::WindowClose;
			break;
		case SDL_WINDOWEVENT:
			//if (sdl_event.window.windowID != Window::APP_WINDOW) return false;
			event->category = EventCategory::WindowEvent;
			if (sdl_event.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				event->category = EventCategory::WindowEvent;
				event->type = EventType::WindowClose;
				return true;
			}
			else if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED || sdl_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				event->type = EventType::WindowResize;
				event->window.width = sdl_event.window.data1;
				event->window.height = sdl_event.window.data2;
			}
			else if (sdl_event.window.event == SDL_WINDOWEVENT_MOVED) {
				event->type = EventType::WindowMove;
				event->window.x = sdl_event.window.data1;
				event->window.y = sdl_event.window.data2;
			}
			else if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) event->type = EventType::WindowGainFocus;
			else if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) event->type = EventType::WindowLostFocus;
			else return false;
			return true;
		case SDL_TEXTINPUT: return false;//From what I understand both of the occured form a button press but only one need it (2 may cause problems)
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0)
				event->type = EventType::KeyPressed;
			else if (sdl_event.type == SDL_KEYUP)
				event->type = EventType::KeyReleased;
			else
				return false;
			event->category = EventCategory::KeyboardEvent;
			event->key.state = (sdl_event.type == SDL_KEYUP) ? RELEASED : PRESSED;
			event->key.code=keymap[sdl_event.key.keysym.sym];
			return true;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			event->category = EventCategory::MouseEvent;
			event->type = (sdl_event.type == SDL_MOUSEBUTTONDOWN) ? EventType::MouseButtonPressed : EventType::MouseButtonReleased;
			event->mouse.button.state = (sdl_event.type == SDL_MOUSEBUTTONDOWN) ? PRESSED : RELEASED;
			if (sdl_event.button.button == SDL_BUTTON_LEFT) event->mouse.button.type = MouseButtonType::Left;
			else if (sdl_event.button.button == SDL_BUTTON_RIGHT) event->mouse.button.type = MouseButtonType::Right;
			else event->mouse.button.type = MouseButtonType::Wheel;
			return true;
		case SDL_MOUSEMOTION:
			event->category = EventCategory::MouseEvent;
			event->type = EventType::MouseMove;
			event->mouse.x = sdl_event.button.x;
			event->mouse.y = sdl_event.button.y;
			return true;
		case SDL_MOUSEWHEEL:
			event->category = EventCategory::MouseEvent;
			event->type = EventType::MouseScroll;
			event->mouse.wheel_dx = sdl_event.wheel.x;
			event->mouse.wheel_dy = sdl_event.wheel.y;
			return true;
		default:
			//GTE_WARN_LOG("Not currently supported Event! SDL_Event type code: ", sdl_event.type);
			return false;
		}
		return true;
	}

}

