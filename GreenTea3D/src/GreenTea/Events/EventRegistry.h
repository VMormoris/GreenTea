#ifndef _EVENT_REGISTRY
#define _EVENT_REGISTRY

#include "Types.h"
#include "Properties.h"
#include "GreenTea/Core/Logger.h"
#pragma warning( push )
#pragma warning( disable : 4267 )
#include <entt.hpp>
#pragma warning( pop )

namespace GTE {

	/**
	* @brief Registry for Event Listeners (Singleton)
	* @details A Event Registry maps Listeners and their callbacks to appropriate events types
	*/
	class ENGINE_API EventRegistry {
	public:

		//As Singleton this class cannot be copied
		EventRegistry(const EventRegistry&) = delete;
		EventRegistry& operator=(const EventRegistry&) = delete;

		/**
		* @brief Removes instance of the object from any type of Events that it may be registered for
		*/
		void RemoveListener(void* ptr);

		/**
		* @brief Getter for the single instance of this class
		*/
		static EventRegistry& Get(void);

	private:

		//As Singleton instances of this class cannot be created
		EventRegistry() = default;

		void MapListener(void* obj_ptr, GTE::EventType eventType);

		/**
		* @brief Objects events map
		* @details A map that contains for every object what events is register for.
		*/
		std::unordered_map<void*, std::vector<EventType>> m_ObjectMap{};

		/**
		* @brief No arguments callbacks map
		* @details A map that contains (for WindowClose, WindownGainFocus and WindowLostFocus events) all the respective callbacks (0 arguments are needed for those events)
		*/
		std::unordered_map<EventType, std::vector<entt::delegate<bool(void)>>> m_VoidCallbacks =
		{
			{EventType::WindowClose, {}},
			{EventType::WindowGainFocus, {}},
			{EventType::WindowLostFocus, {}}
		};

		/**
		* @brief Two integers arguments callbacks map
		* @details A map that contains (for WindowMove, MouseMove and MouseScroll events) all the respective callbacks (2 integers are needed as arguments for those events)
		*/
		std::unordered_map<EventType, std::vector<entt::delegate<bool(int32, int32)>>> m_TwointsCallbacks =
		{
			{EventType::WindowMove, {}},
			{EventType::MouseMove, {}},
			{EventType::MouseScroll, {}}
		};

		/**
		* @brief A KeyCode argument callbacks map
		* @details A map that contains (for KeyPressed and KeyReleased events) all the respective callbacks (1 KeyCode is needed as argument for those events)
		*/
		std::unordered_map<EventType, std::vector<entt::delegate<bool(KeyCode)>>> m_KeycodeCallbacks =
		{
			{EventType::KeyPressed, {}},
			{EventType::KeyReleased, {}}
		};

		/**
		* @brief MouseButtonType argument callbacks map
		* @details A map that contains (for MouseButtonPressed and MouseButtonReleased events) all the respective callbacks (1 MouseButtonType enum is needed as argument for those events)
		*/
		std::unordered_map<EventType, std::vector<entt::delegate<bool(MouseButtonType)>>> m_MousebuttonCallbacks =
		{
			{EventType::MouseButtonPressed, {}},
			{EventType::MouseButtonReleased, {}}
		};

		/**
		* @brief All the callback for Resize events
		*/
		std::vector<entt::delegate<bool(uint32, uint32)>> m_ResizeCallbacks;

		friend class EventDispatcher;

		template<EventType, typename, auto>
		friend struct AddListener;

	};


	/**
	* @brief Functor for binding a Member functions as Event's callbacks on the EventRegistry
	* @tparam eventType Type of the of Event that the member function will be bound for
	* @tparam Class Type of the object that want to bound for an Event
	* @tparam Fn Member function that will be bound as an Event Callback
	*/
	template<EventType eventType, typename Class, auto Fn>
	struct AddListener;

	template<typename Class, auto Fn>
	struct AddListener<EventType::WindowClose, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, EventType::WindowClose);

			entt::delegate<bool(void)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_VoidCallbacks[EventType::WindowClose].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::WindowGainFocus, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, EventType::WindowGainFocus);


			entt::delegate<bool(void)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_VoidCallbacks[EventType::WindowGainFocus].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::WindowLostFocus, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::WindowLostFocus);

			entt::delegate<bool(void)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_VoidCallbacks[EventType::WindowGainFocus].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::WindowResize, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::WindowResize);

	
			entt::delegate<bool(uint32, uint32)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_ResizeCallbacks.push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::WindowMove, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::WindowMove);


			entt::delegate<bool(int32, int32)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_TwointsCallbacks[EventType::WindowMove].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::MouseMove, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::MouseMove);


			entt::delegate<bool(int32, int32)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_TwointsCallbacks[EventType::MouseMove].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::MouseScroll, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::MouseScroll);


			entt::delegate<bool(int32, int32)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_TwointsCallbacks[EventType::MouseScroll].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::MouseButtonPressed, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::MouseButtonPressed);


			entt::delegate<bool(MouseButtonType)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_MousebuttonCallbacks[EventType::MouseButtonPressed].push_back(temp);
		}

	};

	template<typename Class, auto Fn>
	struct AddListener<EventType::MouseButtonReleased, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::MouseButtonReleased);

			entt::delegate<bool(MouseButtonType)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_MousebuttonCallbacks[EventType::MouseButtonReleased].push_back(temp);
		}

	};


	template<typename Class, auto Fn>
	struct AddListener<EventType::KeyPressed, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::KeyPressed);

			entt::delegate<bool(KeyCode)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_KeycodeCallbacks[EventType::KeyPressed].push_back(temp);
		}

	};


	template<typename Class, auto Fn>
	struct AddListener<EventType::KeyReleased, Class, Fn> {

		void operator()(Class* obj_ptr)
		{
			EventRegistry& reg = EventRegistry::Get();
			reg.MapListener(obj_ptr, GTE::EventType::KeyReleased);

			entt::delegate<bool(char)> temp{};
			temp.connect<Fn>(obj_ptr);
			reg.m_KeycodeCallbacks[EventType::KeyReleased].push_back(temp);
		}

	};
}

#define REGISTER(eventType, obj_ptr, function) GTE::AddListener<eventType, std::remove_pointer<decltype(obj_ptr)>::type, function>()(this);
#define UNREGISTER(obj_ptr) GTE::EventRegistry::Get().RemoveListener(obj_ptr)

#endif
