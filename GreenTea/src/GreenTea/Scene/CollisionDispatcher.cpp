#include "CollisionDispatcher.h"
#include "ScriptableEntity.h"
#include "Components.h"

#include <GreenTea/Core/Logger.h>

namespace GTE {

	void CollisionDispatcher::BeginContact(b2Contact* contact)
	{
		Entity A{ static_cast<entt::entity>(contact->GetFixtureA()->GetUserData().pointer), m_Context };
		Entity B{ static_cast<entt::entity>(contact->GetFixtureB()->GetUserData().pointer), m_Context };

		if (A.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = A.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
			{
				ScriptableEntity other;
				other.m_Entity = B;
				nScript.Instance->onCollisionStart(other);
			}
		}
		if (B.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = B.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
			{
				ScriptableEntity other;
				other.m_Entity = A;
				nScript.Instance->onCollisionStart(other);
			}
		}
	}

	void CollisionDispatcher::EndContact(b2Contact* contact)
	{
		Entity A{ static_cast<entt::entity>(contact->GetFixtureA()->GetUserData().pointer), m_Context };
		Entity B{ static_cast<entt::entity>(contact->GetFixtureB()->GetUserData().pointer), m_Context };

		if (A.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = A.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
			{
				ScriptableEntity other;
				other.m_Entity = B;
				nScript.Instance->onCollisionStop(other);
			}
		}
		if (B.HasComponent<NativeScriptComponent>())
		{
			auto& nScript = B.GetComponent<NativeScriptComponent>();
			if (nScript.State == ScriptState::Active)
			{
				ScriptableEntity other;
				other.m_Entity = A;
				nScript.Instance->onCollisionStop(other);
			}
		}
	}

	CollisionDispatcher& CollisionDispatcher::Get(void)
	{
		static CollisionDispatcher dispatcher;
		return dispatcher;
	}

	void CollisionDispatcher::SetContext(Scene* scene) { m_Context = scene; }
		
}