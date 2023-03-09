#include "ScriptableEntity.h"

#include <Engine/Core/Context.h>

static void SpawnFromPrefab(gte::Ref<gte::Asset> prefab, gte::Entity parent = {});
static void SpawnFromPrefabAsync(gte::uuid id, gte::Entity parent);
static void MarkForDestruction(gte::Entity entity);

namespace gte {

	void ScriptableEntity::Start(void) {}
	void ScriptableEntity::Destroy(void) {}

	void ScriptableEntity::FixedUpdate(void) {}
	void ScriptableEntity::Update(float dt) {}

	void ScriptableEntity::onCollisionStart(Entity other) {}
	void ScriptableEntity::onCollisionStop(Entity other) {}

	void SpawnEntity(Ref<Asset> prefab) { SpawnFromPrefab(prefab); }
	void SpawnEntity(Entity parent, Ref<Asset>prefab) { SpawnFromPrefab(prefab, parent); }
	void SpawnEntity(ScriptableEntity* parent, Ref<Asset>prefab)
	{
		uuid id = parent->GetComponent<IDComponent>();
		Entity parentEntity = internal::GetContext()->ActiveScene->FindEntityWithUUID(id, false);
		SpawnFromPrefab(prefab, parentEntity);
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

void SpawnFromPrefab(gte::Ref<gte::Asset> prefab, gte::Entity parent)
{
	using namespace gte;
	prefab = internal::GetContext()->AssetManager.RequestAsset(prefab->ID);
	if (prefab->Type == AssetType::PREFAB)
		internal::GetContext()->ActiveScene->CreateEntityFromPrefab(prefab, parent, false);//Called from update so we already have lock
	else if (prefab->Type == AssetType::INVALID)
	{
		GTE_ERROR_LOG("Try to create prefab with ID: ", prefab->ID, " which doesn't exist.");
		return;
	}
	else if (prefab->Type == AssetType::LOADING)
		SpawnFromPrefabAsync(prefab->ID, parent);
	else
	{
		GTE_ERROR_LOG("Try to create prefab with ID: ", prefab->ID, " but not asset with this ID was found.");
	}
}

void SpawnFromPrefabAsync(gte::uuid id, gte::Entity parent)
{
	using namespace gte;
	std::thread th([](uuid id, Entity parent) {
		Ref<Asset> prefab = internal::GetContext()->AssetManager.RequestAsset(id);
		while (prefab->Type == AssetType::LOADING)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			prefab = internal::GetContext()->AssetManager.RequestAsset(id);
		}
		if (prefab->Type != AssetType::PREFAB)
		{
			GTE_ERROR_LOG("Try to create prefab with ID: ", prefab->ID, " but not asset with this ID was found.");
			return;
		}
		internal::GetContext()->ActiveScene->CreateEntityFromPrefab(prefab, parent);
	}, id, parent);
	th.detach();
}