#pragma once
#include <Engine/Scene/Entity.h>

namespace gte {

	class ENGINE_API ScriptableEntity {
	public:
		ScriptableEntity(void) = default;
		ScriptableEntity(Entity entity)
			: mEntity(entity) {}

		virtual void Start(void);
		virtual void Destroy(void);

		virtual void FixedUpdate(void);
		virtual void Update(float dt);

		virtual void onCollisionStart(Entity other);
		virtual void onCollisionStop(Entity other);

		template<typename T>
		[[nodiscard]] T& GetComponent(void) { return mEntity.GetComponent<T>(); }

		template<typename T>
		[[nodiscard]] bool HasComponent(void) const { return mEntity.HasComponent<T>(); }

		template<typename T, typename ...Args>
		T& AddComponent(Args&& ...args) { return mEntity.AddComponent<T>(std::forward<Args>(args)...); }

		template<typename T>
		void RemoveComponent(void) { mEntity.RemoveComponent<T>(); }

	private:
		Entity mEntity;
		friend class Scene;
	};

}