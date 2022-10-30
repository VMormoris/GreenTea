#pragma once
#include <Engine/Scene/Entity.h>

namespace gte {

	class ENGINE_API ScriptableEntity {
	public:
		ScriptableEntity(void) = default;
		ScriptableEntity(Entity entity)
			: mEntity(entity) {}
		virtual ~ScriptableEntity(void) = default;

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

	/**
	* @brief Creates a new entity base on a Prefab
	* @param prefab A reference to a Prefab-Asset that describes the entity that will be spawned
	*/
	ENGINE_API void SpawnEntity(Ref<Asset> prefab);
	
	/**
	* @brief Creates a new entity base on a Prefab attached as a child to the given Entity
	* @param parent Entity object that the newly created entity will be attached to
	* @param prefab A reference to a Prefab-Asset that describes the entity that will be spawned
	*/
	ENGINE_API void SpawnEntity(Entity parent, Ref<Asset>prefab);

	/**
	* @brief Creates a new entity base on a Prefab attached as a child to the given Entity
	* @param parent Pointer to a ScriptableEntity that the newly created entity will be attached to
	* @param prefab A reference to a Prefab-Asset that describes the entity that will be spawned
	*/
	ENGINE_API void SpawnEntity(ScriptableEntity* parent, Ref<Asset>prefab);

	/**
	* @brief Destroys the specified Entity on this or the next frame (if has ScriptComponent)
	* @param entity Entity object that will be destroyed
	*/
	ENGINE_API void DestroyEntity(Entity entity);

	/**
	* @brief Destroys the specified ScriptableEntity on the frame
	* @param entity Pointer to a ScriptableEntity that will be destroyed
	*/
	ENGINE_API void DestroyEntity(ScriptableEntity* entity);

	/**
	* @brief Get all entities with the specified tag (aka name)
	* @param tag String containing the name you are searching for
	* @return A vector containing all the entities with the specified tag
	*/
	ENGINE_API [[nodiscard]] std::vector<Entity> GetEntitiesByTag(const std::string& tag);

}