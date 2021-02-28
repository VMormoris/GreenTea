#ifndef _SCRIPTABLE_ENTITY
#define _SCRIPTABLE_ENTITY

#include "Entity.h"

namespace GTE {

	class ENGINE_API ScriptableEntity {
	public:

		virtual ~ScriptableEntity(void) = default;

		virtual void Start(void);
		virtual void Destroy(void);

		virtual void FixedUpdate(void);
		virtual void Update(float dt);

		virtual void onCollisionStart(ScriptableEntity other);
		virtual void onCollisionStop(ScriptableEntity other);
		
		template<typename T>
		T& GetComponent(void) { return m_Entity.GetComponent<T>(); }

		template<typename T, typename ...Args>
		T& AddComponent(Args&& ...args) { return m_Entity.AddComponent<T, Args>(std::forward<Args>(args)...); }

		template<typename T>
		bool HasComponent(void) { return m_Entity.HasComponent<T>(); }

		template<typename T>
		void RemoveComponent(void) { m_Entity.RemoveComponent<T>(); }

	private:
		Entity m_Entity;
		friend class Scene;
		friend class CollisionDispatcher;
	};

}
#endif