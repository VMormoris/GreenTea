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

}