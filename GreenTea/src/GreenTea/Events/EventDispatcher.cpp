#include "EventDispatcher.h"
#include "EventRegistry.h"

namespace GTE {

	void EventDispatcher::Dispatch(const Event& event)
	{
		EventType eventType = event.type;
		EventRegistry& registry = EventRegistry::Get();
		switch (eventType)
		{
		case EventType::WindowClose:
		case EventType::WindowGainFocus:
		case EventType::WindowLostFocus:
			for (auto cit = registry.m_VoidCallbacks[eventType].cbegin(); cit != registry.m_VoidCallbacks[eventType].cend(); ++cit)
			{
				if ((*cit)()) break;
			}
			break;
		case EventType::WindowMove:
			for (auto cit = registry.m_TwointsCallbacks[eventType].cbegin(); cit != registry.m_TwointsCallbacks[eventType].cend(); ++cit)
			{
				if ((*cit)(event.window.x, event.window.y)) break;
			}
			break;
		case EventType::MouseMove:
			for (auto cit = registry.m_TwointsCallbacks[eventType].cbegin(); cit != registry.m_TwointsCallbacks[eventType].cend(); ++cit)
			{
				if ((*cit)(event.mouse.x, event.mouse.y)) break;
			}
			break;
		case EventType::MouseScroll:
			for (auto cit = registry.m_TwointsCallbacks[eventType].cbegin(); cit != registry.m_TwointsCallbacks[eventType].cend(); ++cit)
			{
				if ((*cit)(event.mouse.wheel_dx, event.mouse.wheel_dy)) break;
			}
			break;
		case EventType::KeyPressed:
		case EventType::KeyReleased:
			for (auto cit = registry.m_KeycodeCallbacks[eventType].cbegin(); cit != registry.m_KeycodeCallbacks[eventType].cend(); ++cit)
			{
				if ((*cit)(event.key.code)) break;
			}
			break;
		case EventType::MouseButtonPressed:
		case EventType::MouseButtonReleased:
			for (auto cit = registry.m_MousebuttonCallbacks[eventType].cbegin(); cit != registry.m_MousebuttonCallbacks[eventType].cend(); ++cit)
			{
				if ((*cit)(event.mouse.button.type)) break; 
			}
			break;
		case EventType::WindowResize:
			for (auto cit = registry.m_ResizeCallbacks.cbegin(); cit != registry.m_ResizeCallbacks.cend(); ++cit)
			{
				if ((*cit)(event.window.width, event.window.height)) break;
			}
			break;
		default:
			ENGINE_ASSERT(false, "Not valid EventType!");
			break;
		}
	}

}