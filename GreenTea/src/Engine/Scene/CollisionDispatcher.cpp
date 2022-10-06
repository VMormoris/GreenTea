#include "CollisionDispatcher.h"
#include "Components.h"

#include <Engine/Core/Context.h>
#include <Engine/NativeScripting/ScriptableEntity.h>

#include <box2d/b2_contact.h>

namespace gte {

	void CollisionDispatcher::BeginContact(b2Contact* contact)
	{
		Scene* scene = internal::GetContext()->ActiveScene;
		Entity A{ static_cast<entt::entity>(contact->GetFixtureA()->GetUserData().pointer), scene };
		Entity B{ static_cast<entt::entity>(contact->GetFixtureB()->GetUserData().pointer), scene };

		if (A.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = A.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
				nScript.Instance->onCollisionStart(B);
		}
		if (B.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = B.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
				nScript.Instance->onCollisionStart(A);
		}
	}

	void CollisionDispatcher::EndContact(b2Contact* contact)
	{
		Scene* scene = internal::GetContext()->ActiveScene;
		Entity A{ static_cast<entt::entity>(contact->GetFixtureA()->GetUserData().pointer), scene };
		Entity B{ static_cast<entt::entity>(contact->GetFixtureB()->GetUserData().pointer), scene };

		if (A.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = A.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
				nScript.Instance->onCollisionStop(B);
		}
		if (B.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = B.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
				nScript.Instance->onCollisionStop(A);
		}
	}

	CollisionDispatcher* CollisionDispatcher::Get(void)
	{
		if (!internal::GetContext()->CDispatcher)
			internal::GetContext()->CDispatcher = new CollisionDispatcher();
		return internal::GetContext()->CDispatcher;
	}


}