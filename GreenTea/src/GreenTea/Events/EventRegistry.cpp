#include "EventRegistry.h"

namespace GTE {

	void EventRegistry::RemoveListener(void* ptr)
	{
		std::vector<EventType> events = m_ObjectMap[ptr];
		for (EventType eventType : events)
		{
			switch (eventType)
			{
			case EventType::WindowClose:
			case EventType::WindowGainFocus:
			case EventType::WindowLostFocus:
				for (auto cit = m_VoidCallbacks[eventType].cbegin(); cit != m_VoidCallbacks[eventType].cend(); ++cit)
				{
					if ((*cit).instance() == ptr) { m_VoidCallbacks[eventType].erase(cit); break; }
				}
				break;
			case EventType::WindowMove:
			case EventType::MouseMove:
			case EventType::MouseScroll:
				for (auto cit = m_TwointsCallbacks[eventType].cbegin(); cit != m_TwointsCallbacks[eventType].cend(); ++cit)
				{
					if ((*cit).instance() == ptr) { m_TwointsCallbacks[eventType].erase(cit); break; }
				}
				break;
			case EventType::KeyPressed:
			case EventType::KeyReleased:
				for (auto cit = m_KeycodeCallbacks[eventType].cbegin(); cit != m_KeycodeCallbacks[eventType].cend(); ++cit)
				{
					if ((*cit).instance() == ptr) { m_KeycodeCallbacks[eventType].erase(cit); break; }
				}
				break;
			case EventType::MouseButtonPressed:
			case EventType::MouseButtonReleased:
				for (auto cit = m_MousebuttonCallbacks[eventType].cbegin(); cit != m_MousebuttonCallbacks[eventType].cend(); ++cit)
				{
					if ((*cit).instance() == ptr) { m_MousebuttonCallbacks[eventType].erase(cit); break; }
				}
				break;
			case EventType::WindowResize:
				for (auto cit = m_ResizeCallbacks.cbegin(); cit != m_ResizeCallbacks.cend(); ++cit)
				{
					if ((*cit).instance() == ptr) { m_ResizeCallbacks.erase(cit); break; }
				}
				break;
			default:
				ENGINE_ASSERT(false, "Not valid EventType!");
				break;
			}
			m_ObjectMap.erase(ptr);
		}
	}

	void EventRegistry::MapListener(void* ptr, GTE::EventType eventType)
	{
		if (m_ObjectMap.find(ptr) == m_ObjectMap.end())
			m_ObjectMap.insert({ ptr, {} });
		m_ObjectMap[ptr].push_back(eventType);
	}


	EventRegistry& EventRegistry::Get(void)
	{
		static EventRegistry instance;
		return instance;
	}

}