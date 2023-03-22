#pragma once
#include "Components.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/uuid.h>
#include <Engine/GPU/FrameBuffer.h>

#include <entt.hpp>
#include <glm.hpp>

//Forward declaration(s)
class b2World;

namespace gte {

	//Forward declaration(s)
	class Entity;
	namespace internal { class SceneSerializer; }

	/**
	* @brief Container of Game Objects (Entities)
	*/
	class ENGINE_API Scene {
	public:
		//Constructor(s) & Destructor
		Scene(void);
		~Scene(void) {}

		void Update(float dt);
		void Render(const glm::mat4& eyematrix);

		void OnViewportResize(uint32 width, uint32 height);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(const uuid& id, const std::string& name = std::string());
		Entity CreateChildEntity(Entity parent);
		Entity CreateEntityFromPrefab(Ref<Asset> prefab, Entity parent);
		void MoveEntity(Entity parent, Entity toMove);
		Entity Clone(Entity toClone, bool recursive = false);
		void DestroyEntity(Entity entity);

		[[nodiscard]] Entity FindEntityWithUUID(const uuid& id);
		[[nodiscard]] std::vector<Entity> GetEntitiesByTag(const std::string& tag);
		[[nodiscard]] Entity GetPrimaryCameraEntity(void);

		void UpdateTransform(Entity entity);
		void UpdateMatrices(void);//Update Transform for all entities

		void OnStart(void);
		void OnStop(void);

		void DestroyRuntime(void);
		void PatchScripts(void);

		[[nodiscard]] static Scene* Copy(Scene* other);

		template<typename ...Components>
		[[nodiscard]] auto GetAllEntitiesWith(void) { return mReg.view<Components...>(); } 

	private:

		void OnPhysicsStart(void);
		void OnPhysicsStop(void);
		void SetupPhysics(void);
		void FixedUpdate(void);
		void Movement(float dt, bool physics);

		void SetupUserComponents(entt::entity entityID, const UserDefinedComponents& udc);

		void InformAudioEngine(void);
		void InformEngine(entt::entity entityID, Rigidbody2DComponent& rb, TransformationComponent& tc);
		void InformPhysicsWorld(Rigidbody2DComponent& rb, Collider* collider, const glm::vec3& pos, float angle);
		//All Components expect relationship that needs special handling
		// it also assumes that destination entity is "Empty Enity" (aka has only ID, Tag and Relationship component)
		void CopyComponents(Entity source, Entity destination);

	private:
		entt::registry mReg;
		entt::registry mPhysicsReg;
		b2World* mPhysicsWorld = nullptr;
		float mAccumulator = 0.0f;
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class internal::SceneSerializer;
	};
}