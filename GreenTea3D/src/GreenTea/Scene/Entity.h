#ifndef _ENTITY
#define _ENTITY

#include "GreenTea/Core/EngineCore.h"
#include "GreenTea/Core/Logger.h"
#include "Scene.h"

namespace GTE {

	class ENGINE_API Entity {
	public:

		Entity(void) = default;
		Entity(const Entity& other) = default;

		Entity(entt::entity entity, Scene* owner)
			: m_Entity(entity), m_Owner(owner) {}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ASSERT(!HasComponent<T>(), "Entity already has component!");
			return m_Owner->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent(void)
		{
			ASSERT(HasComponent<T>(), "Entity does not have this component!");
			return m_Owner->m_Registry.get<T>(m_Entity);
		}

		template<typename T>
		const T& GetComponent(void) const
		{
			ASSERT(HasComponent<T>(), "Entity does not have this component!");
			return m_Owner->m_Registry.get<T>(m_Entity);
		}

		template<typename T>
		bool HasComponent(void) const
		{
			return m_Owner->m_Registry.has<T>(m_Entity);
		}

		template<typename T>
		void RemoveComponent(void)
		{
			ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Owner->m_Registry.remove<T>(m_Entity);
		}

		void Destroy(void);

		bool Valid(void) const;

		Entity Clone(bool recursive = false);
		
		void AddChild(void);

		void UpdateMatrices(void);

		bool operator==(const Entity& rhs) { return (m_Entity == rhs.m_Entity) && (m_Owner == rhs.m_Owner); }
		bool operator!=(const Entity& rhs) { return (m_Entity != rhs.m_Entity) || (m_Owner != rhs.m_Owner); }
	
	private:

		entt::entity m_Entity { entt::null };
		Scene* m_Owner = nullptr;
	};

}

#endif