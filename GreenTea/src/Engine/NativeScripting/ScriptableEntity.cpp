#include "ScriptableEntity.h"

#include <Engine/Core/Context.h>

static gte::Entity SpawnFromPrefab(gte::Ref<gte::Asset> prefab, gte::Entity parent = {});
static void MarkForDestruction(gte::Entity entity);

namespace gte {

	void ScriptableEntity::Start(void) {}
	void ScriptableEntity::Destroy(void) {}

	void ScriptableEntity::FixedUpdate(void) {}
	void ScriptableEntity::Update(float dt) {}

	void ScriptableEntity::onCollisionStart(Entity other) {}
	void ScriptableEntity::onCollisionStop(Entity other) {}

	Entity SpawnEntity(Ref<Asset> prefab) { return SpawnFromPrefab(prefab); }
	Entity SpawnEntity(Entity parent, Ref<Asset>prefab) { return SpawnFromPrefab(prefab, parent); }
	Entity SpawnEntity(ScriptableEntity* parent, Ref<Asset>prefab)
	{
		Entity parentEntity = { (entt::entity)(uint32)*parent, internal::GetContext()->ActiveScene };
		return SpawnFromPrefab(prefab, parentEntity);
	}

	void DestroyEntity(ScriptableEntity* entity)
	{
		entity->GetComponent<NativeScriptComponent>().State = ScriptState::MustBeDestroyed;

		Scene* const scene = internal::GetContext()->ActiveScene;
		const auto& rel = entity->GetComponent<RelationshipComponent>();
		Entity child = { rel.FirstChild, scene };
		for (size_t i = 0; i < rel.Childrens; i++)
		{
			MarkForDestruction(child);
			const auto& crel = child.GetComponent<RelationshipComponent>();
			child = { crel.Next, scene };
		}

		entity->AddComponent<filters::Destructable>();
	}

	void DestroyEntity(Entity entity) { MarkForDestruction(entity); }

	[[nodiscard]] std::vector<Entity> GetEntitiesByTag(const std::string& tag) { return internal::GetContext()->ActiveScene->GetEntitiesByTag(tag); }

}

void MarkForDestruction(gte::Entity entity)
{
	using namespace gte;
	if (entity.HasComponent<NativeScriptComponent>())
	{
		auto& nsc = entity.GetComponent<NativeScriptComponent>();
		nsc.State = ScriptState::MustBeDestroyed;
	}

	Scene* const scene = internal::GetContext()->ActiveScene;
	const auto& rel = entity.GetComponent<RelationshipComponent>();
	Entity child = { rel.FirstChild, scene };
	for (size_t i = 0; i < rel.Childrens; i++)
	{
		MarkForDestruction(child);
		const auto& crel = child.GetComponent<RelationshipComponent>();
		child = { crel.Next, scene};
	}

	entity.AddComponent<filters::Destructable>();
}

gte::Entity SpawnFromPrefab(gte::Ref<gte::Asset> prefab, gte::Entity parent)
{
	using namespace gte;
	
	prefab = internal::GetContext()->AssetManager.RequestAsset(prefab->ID);
	while (prefab->Type == AssetType::LOADING)//Wait for prefab to be loaded
		prefab = internal::GetContext()->AssetManager.RequestAsset(prefab->ID);

	if (prefab->Type != AssetType::PREFAB)//Check if the requested ID coresponds to a prefab
	{
		GTE_ERROR_LOG("ID: ", prefab->ID, " doesn't corespond to a valid prefab.");
		return {};
	}

	return internal::GetContext()->ActiveScene->CreateEntityFromPrefab(prefab, parent);
}