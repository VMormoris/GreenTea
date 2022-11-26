#include "Entity.h"
#include "Components.h"

namespace gte {

	Entity::Entity(void) {}

	[[nodiscard]] uuid Entity::GetID(void) { return mEntityHandle == entt::null ? uuid{} : GetComponent<IDComponent>().ID; }
	
	[[nodiscard]] std::string Entity::GetName(void) { return mEntityHandle == entt::null ? std::string{} : GetComponent<TagComponent>().Tag; }

}