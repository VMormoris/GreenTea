#pragma once
#include <Engine/Scene/Scene.h>

namespace gte {
	
	class ENGINE_API System {
	public:
		System(void) = default;
		virtual ~System(void) = default;

	protected:

		virtual void onFixedUpdateStart(void);
		virtual void onFixedUpdateEnd(void);

		virtual void onUpdateStart(float dt);
		virtual void onUpdateEnd(void);

		template<typename Component, typename ...Other, typename ...Exclude>
		auto View(entt::exclude_t<Exclude...> excluded = {}) { return mReg->view<Component, Other...>(excluded); }

		template<typename Component, typename ...Other, typename ...Exclude>
		auto View(entt::exclude_t<Exclude...> excluded = {}) const { return mReg->view<Component, Other...>(excluded); }

		//template<typename ...Owned, typename ...Exclude>
		//auto Group(entt::exclude_t<Exclude...> excluded = {}) { return mReg->group<Owned...>(excluded); }

		//template<typename ...Owned, typename ...Get, typename ...Exclude>
		//auto Group(entt::get_t<Get...> gotten = {}, entt::exclude_t<Exclude...> excluded = {}) { return mReg->group<Owned...>(gotten, excluded); }

	private:
		entt::registry* mReg = nullptr;
		friend class Scene;
	};

}