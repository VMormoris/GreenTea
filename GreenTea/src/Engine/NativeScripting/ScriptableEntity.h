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

		operator uint32(void) const { return (uint32)mEntity; }

	private:
		Entity mEntity;
		friend class Scene;
	};

	ENGINE_API void SpawnEntity(Ref<Asset> prefab);
	ENGINE_API void SpawnEntity(Entity parent, Ref<Asset>prefab);
	ENGINE_API void SpawnEntity(ScriptableEntity* parent, Ref<Asset>prefab);

	ENGINE_API void DestroyEntity(Entity entity);
	ENGINE_API void DestroyEntity(ScriptableEntity* entity);
}