#include "Entity.h"
#include "Components.h"

namespace gte {

	[[nodiscard]] uuid Entity::GetID(void) { return mEntityHandle == entt::null ? uuid{} : GetComponent<IDComponent>().ID; }
	
	[[nodiscard]] std::string Entity::GetName(void) { return mEntityHandle == entt::null ? std::string{} : GetComponent<TagComponent>().Tag; }

	void Entity::SetActive(bool value)
	{
		if (value && HasComponent<filters::Disabled>())
			RemoveComponent<filters::Disabled>();
		else if (!value && !HasComponent<filters::Disabled>())
			AddComponent<filters::Disabled>();

		const auto& rel = GetComponent<RelationshipComponent>();
		Entity child = { rel.FirstChild, mOwner };
		for (size_t i = 0; i < rel.Childrens; i++)
		{
			child.SetActive(value);
			const auto& crel = child.GetComponent<RelationshipComponent>();
			child = { crel.Next, mOwner };
		}
	}

	Entity Entity::GetParent(void) noexcept { return { GetComponent<RelationshipComponent>().Parent , mOwner }; }

	std::vector<Entity> Entity::GetChildrens(void) noexcept
	{
		const auto& relc = GetComponent<RelationshipComponent>();
		std::vector<Entity> childrens;
		childrens.reserve(relc.Childrens);

		Entity child = { relc.FirstChild, mOwner };
		for (size_t i = 0; i < relc.Childrens; i++)
		{
			childrens.emplace_back(child);
			const auto& crel = child.GetComponent<RelationshipComponent>();
			child = { crel.Next, mOwner };
		}

		return childrens;
	}

	Entity Entity::GetChild(size_t index)
	{
		const auto& relc = GetComponent<RelationshipComponent>();
		ASSERT(index >= relc.Childrens || index < 0, "Index: ", index, " is out of bounds.");

		Entity child = { relc.FirstChild, mOwner };
		for (size_t i = 0; i < index; i++)
		{
			const auto& crel = child.GetComponent<RelationshipComponent>();
			child = { crel.Next, mOwner };
		}

		return child;
	}

}