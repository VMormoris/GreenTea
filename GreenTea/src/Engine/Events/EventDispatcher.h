#pragma once

#include "Event.h"

#include <Engine/NativeScripting/ScriptableEntity.h>

#include <entt.hpp>

namespace gte::internal {

	template<EventType type>
	struct basic_signal;
	template<>
	struct basic_signal<EventType::WindowClose> { typedef typename entt::sigh<bool(void)> Type; };
	template<>
	struct basic_signal<EventType::WindowResize> { typedef typename entt::sigh<bool(uint32, uint32)> Type; };
	template<>
	struct basic_signal<EventType::WindowMove> { typedef typename entt::sigh<bool(int32, int32)> Type; };
	template<>
	struct basic_signal<EventType::WindowGainFocus> { typedef typename entt::sigh<bool(void)> Type; };
	template<>
	struct basic_signal<EventType::WindowLostFocus> { typedef typename entt::sigh<bool(void)> Type; };
	template<>
	struct basic_signal<EventType::KeyPressed> { typedef typename entt::sigh<bool(KeyCode)> Type; };
	template<>
	struct basic_signal<EventType::KeyReleased> { typedef typename entt::sigh<bool(KeyCode)> Type; };
	template<>
	struct basic_signal<EventType::MouseButtonPressed> { typedef typename entt::sigh<bool(MouseButtonType)> Type; };
	template<>
	struct basic_signal<EventType::MouseButtonReleased> { typedef typename entt::sigh<bool(MouseButtonType)> Type; };
	template<>
	struct basic_signal<EventType::MouseMove> { typedef typename entt::sigh<bool(int32, int32)> Type; };
	template<>
	struct basic_signal<EventType::MouseScroll> { typedef typename entt::sigh<bool(float, float)> Type; };

	template<EventType type>
	using signal_t = typename basic_signal<type>::Type;


	// Base on skypjack's entt:dispatcher
	// can be found here: https://github.com/skypjack/entt/blob/master/src/entt/signal/dispatcher.hpp
	class ENGINE_API EventDispatcher {

		struct basic_pool {
			virtual ~basic_pool(void) = default;
			virtual void disconnect(void*) = 0;
			[[nodiscard]] virtual size_t size(void) const noexcept = 0;
		};
		
		template<EventType type>
		struct pool final : public basic_pool {
			using sink_type = typename signal_t<type>::sink_type;
			void disconnect(void* instance) { sink.disconnect(instance); }
			[[nodiscard]] size_t size(void) const noexcept { return signal.size(); }
			
			signal_t<type> signal;
			sink_type sink{ signal };
		};
	
	public:

		template<EventType type, typename T, auto Func>
		void AddListener(T& instance)
		{
			if constexpr (type == EventType::Click || type == EventType::HoverEnter || type == EventType::HoverExit)
			{
				entt::delegate<bool(void)> temp;
				temp.template connect<Func>(instance);
				mSpecialPool.emplace(std::make_pair(&instance, type), temp);
			}
			else
			{
				auto listeners = static_cast<pool<type>*>(mPools[index<type>()]);
				listeners->sink.template connect<Func>(instance);
			}
		}

		template<typename T>
		void RemoveListener(T* instance)
		{
			for (auto pool : mPools)
				pool->disconnect(instance);

			for (auto it = mSpecialPool.begin(); it != mSpecialPool.end();)
			{
				if ((*it).first.first == (ScriptableEntity*)instance)
					mSpecialPool.erase(it++);
				else
					++it;
			}
		}

		template<EventType type, typename ...Args>
		void Dispatch(Args&& ...args)
		{
			auto listeners = static_cast<pool<type>*>(mPools[index<type>()]);
			listeners->signal.collect([](bool handled) { return handled; }, std::forward<Args>(args)...);
		}

		template<EventType type, typename ...Args>
		void Dispatch(Entity entity, Args&& ...args)
		{
			if (!entity.HasComponent<NativeScriptComponent>())
				return;

			ScriptableEntity* se = entity.GetComponent<NativeScriptComponent>().Instance;
			auto key = std::make_pair(se, type);
			if(mSpecialPool.find(key) != mSpecialPool.end())
				mSpecialPool[key](std::forward<Args>(args)...);
		}

	private:

		template<EventType type>
		static constexpr size_t index(void) noexcept
		{
			if constexpr (type == EventType::WindowClose)				return 0;
			else if constexpr (type == EventType::WindowResize)			return 1;
			else if constexpr (type == EventType::WindowMove)			return 2;
			else if constexpr (type == EventType::WindowGainFocus)		return 3;
			else if constexpr (type == EventType::WindowLostFocus)		return 4;
			else if constexpr (type == EventType::KeyPressed)			return 5;
			else if constexpr (type == EventType::KeyReleased)			return 6;
			else if constexpr (type == EventType::MouseButtonPressed)	return 7;
			else if constexpr (type == EventType::MouseButtonReleased)	return 8;
			else if constexpr (type == EventType::MouseMove)			return 9;
			else if constexpr (type == EventType::MouseScroll)			return 10;
		}

	private:

		std::array<basic_pool*, 11> mPools =
		{
			//Pools for window events
			new pool<EventType::WindowClose>(),
			new pool<EventType::WindowResize>(),
			new pool<EventType::WindowMove>(),
			new pool<EventType::WindowGainFocus>(),
			new pool<EventType::WindowLostFocus>(),
			//Pools for Keyboard events
			new pool<EventType::KeyPressed>(),
			new pool<EventType::KeyReleased>(),
			//Pools for Mouse events
			new pool<EventType::MouseButtonPressed>(),
			new pool<EventType::MouseButtonReleased>(),
			new pool<EventType::MouseMove>(),
			new pool<EventType::MouseScroll>()
		};

		std::map<std::pair<ScriptableEntity*, EventType>, entt::delegate<bool(void)>> mSpecialPool;
	};
	
}

/**
* @brief Registers a method for the specified event type
* @param eventType EventType that you want to register for
* @param obj_ptr Pointer to the object that will register for the event
* @param function Method that will be used to consume the event
* @warning Call UNREGISTER(obj_ptr) when you don't wont to handle the event anymore
*/
#define REGISTER(eventType, obj_ptr, function) gte::internal::GetContext()->Dispatcher.AddListener<eventType, std::remove_pointer<decltype(obj_ptr)>::type, function>(*obj_ptr)

/**
* @brief Unregisters the specified object for all EventTypes that is register for.
* @param obj_ptr Pointer to the object that will be unregister for all EventTypes
*/
#define UNREGISTER(obj_ptr) gte::internal::GetContext()->Dispatcher.RemoveListener<std::remove_pointer<decltype(obj_ptr)>::type>(obj_ptr)