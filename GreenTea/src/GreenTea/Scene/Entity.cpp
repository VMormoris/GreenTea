#include "Entity.h"
#include "Components.h"

#include <gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

namespace GTE {

	//Aquiring a reference before adding Component may cause undefined behaviour
	Entity Entity::Clone(bool recursive)
	{
		auto entity = m_Owner->m_Registry.create();
		Entity newEntity = { entity, m_Owner };
		if (HasComponent<TagComponent>())
		{
			auto& tag = newEntity.AddComponent<TagComponent>();
			tag.Tag = GetComponent<TagComponent>().Tag;
		}
		if (HasComponent<TransformComponent>())
		{
			auto& transform = newEntity.AddComponent<TransformComponent>();
			auto& ref = GetComponent<TransformComponent>();
			transform.Position = ref.Position;
			transform.Scale = ref.Scale;
			transform.Rotation = ref.Rotation;
		}
		if (HasComponent<Renderable2DComponent>())
		{
			auto& renderable = newEntity.AddComponent<Renderable2DComponent>();
			auto& ref = GetComponent<Renderable2DComponent>();
			renderable = ref;
		}
		if (HasComponent<PerspectiveCameraComponent>())
		{
			auto& camProp = newEntity.AddComponent<PerspectiveCameraComponent>();
			auto& ref = GetComponent<PerspectiveCameraComponent>();
			camProp.Target = ref.Target;
			camProp.UpVector = ref.UpVector;
			camProp.FoV = ref.FoV;
			camProp.Near = ref.Near;
			camProp.Far = ref.Far;
		}
		if (HasComponent<CameraComponent>())
		{
			auto& cam = newEntity.AddComponent<CameraComponent>();
			auto& ref = GetComponent<CameraComponent>();
			cam = ref;
		}
		if (HasComponent<NativeScriptComponent>())
		{
			 auto& nScript = newEntity.AddComponent<NativeScriptComponent>();
			 auto& ref = GetComponent<NativeScriptComponent>();
			 nScript.ClassName = ref.ClassName;
			 nScript.State = ScriptState::MustBeInitialized;
		}
		if (HasComponent<RelationshipComponent>())
		{
			const auto relationship = GetComponent<RelationshipComponent>();
			auto& rel = newEntity.AddComponent<RelationshipComponent>();
			rel.Childrens = relationship.Childrens;
			Entity Parent = { relationship.Parent, m_Owner };
			if (Parent.Valid()&&!recursive)
			{
				rel.Parent = relationship.Parent;
				auto& prel = Parent.GetComponent<RelationshipComponent>();
				prel.Childrens += 1;
				rel.Next = prel.FirstChild;
				prel.FirstChild = entity;
				Entity OldChild = { rel.Next, m_Owner };
				OldChild.GetComponent<RelationshipComponent>().Previous = entity;
			}

			Entity CurrChild = { relationship.FirstChild, m_Owner };
			Entity OldChild = { entt::null, nullptr };
			if (CurrChild.Valid())
			{
				Entity Child = CurrChild.Clone(true);
				auto& crel = Child.GetComponent<RelationshipComponent>();
				crel.Parent = entity;
				newEntity.GetComponent<RelationshipComponent>().FirstChild = Child.m_Entity;
				//rel.FirstChild = Child._entity;
				OldChild = Child;
			}
			for (size_t i = 1; i < relationship.Childrens; i++)
			{
				Entity PrevChild = CurrChild;
				const auto prel = PrevChild.GetComponent<RelationshipComponent>();
				CurrChild = { prel.Next, m_Owner };
				Entity Child = CurrChild.Clone(true);
				auto& crel = Child.GetComponent<RelationshipComponent>();
				crel.Parent = entity;
				crel.Previous = OldChild.m_Entity;
				auto& orel = OldChild.GetComponent<RelationshipComponent>();
				orel.Next = Child.m_Entity;
				OldChild = Child;
			}
		}
		return newEntity;
	}

	void Entity::AddChild(void)
	{
		Entity Child = m_Owner->CreateEntity();
		auto& rel = GetComponent<RelationshipComponent>();
		entt::entity ochild = rel.FirstChild;
		rel.FirstChild = Child.m_Entity;
		rel.Childrens += 1;
		auto& crel = Child.GetComponent<RelationshipComponent>();
		crel.Parent = m_Entity;
		Entity OldChild = { ochild, m_Owner };
		if (OldChild.Valid())
		{
			crel.Next = ochild;
			auto& ocrel = OldChild.GetComponent<RelationshipComponent>();
			ocrel.Previous = Child.m_Entity;
		}
	}

	void Entity::Destroy(void)
	{
		auto& rel = GetComponent<RelationshipComponent>();
		size_t childrens = rel.Childrens;
		Entity Child = { rel.FirstChild, m_Owner };
		for (size_t i = 0; i < childrens; i++)
		{
			auto& childrel = Child.GetComponent<RelationshipComponent>();
			Entity NextChild = { childrel.Next, m_Owner };
			Child.Destroy();
			Child = NextChild;
		}
		Entity Parent = { rel.Parent, m_Owner };
		if (Parent.Valid())
		{
			auto nchild = rel.Next;
			auto pchild = rel.Previous;

			auto& prel = Parent.GetComponent<RelationshipComponent>();
			Entity NextChild = { rel.Next, m_Owner };
			if (prel.FirstChild == m_Entity && prel.Childrens == 1)
				prel.FirstChild = entt::null;
			else if (prel.FirstChild == m_Entity)
			{
				prel.FirstChild = nchild;
				if (NextChild.Valid())
				{
					auto& nrel = NextChild.GetComponent<RelationshipComponent>();
					nrel.Previous = entt::null;
				}
			}
			else
			{
				if (NextChild.Valid())
				{
					auto& nrel = NextChild.GetComponent<RelationshipComponent>();
					nrel.Previous = pchild;
				}
				Entity PreviousChild = { pchild, m_Owner };
				auto& prrel = PreviousChild.GetComponent<RelationshipComponent>();
				prrel.Next = nchild;
			}
			prel.Childrens -= 1;
		}
		m_Owner->m_Registry.destroy(m_Entity);
		m_Entity = entt::null;
		m_Owner = nullptr;
	}

	bool Entity::Valid(void) const
	{
		if (m_Entity == entt::null || m_Owner == nullptr) return false;
		return m_Owner->m_Registry.valid(m_Entity);
	}

	void Entity::UpdateMatrices(void)
	{
		const auto& rel = GetComponent<RelationshipComponent>();
		if (HasComponent<TransformationComponent>())
		{
			const auto& transform = GetComponent<TransformComponent>();
			auto& transformation = GetComponent<TransformationComponent>();

			transformation = glm::translate(glm::mat4(1.0f), transform.Position) *
				glm::toMat4(glm::quat(glm::radians(transform.Rotation))) *
				glm::scale(glm::mat4(1.0f), transform.Scale);

			if (rel.Parent != entt::null)
			{
				if (m_Owner->m_Registry.has<TransformationComponent>(rel.Parent))
				{
					const auto& ptransformation = m_Owner->m_Registry.get<TransformationComponent>(rel.Parent);
					transformation = ptransformation.Transform * transformation.Transform;
				}
			}

			if (HasComponent<CameraComponent>())
			{
				auto& camera = GetComponent<CameraComponent>();
				auto& persp = GetComponent<PerspectiveCameraComponent>();
				camera.ViewMatrix = glm::lookAt(glm::vec3(transformation.Transform[3]), persp.Target, persp.UpVector);
				camera.EyeMatrix = camera.ProjectionMatrix * camera.ViewMatrix;
			}

			if (HasComponent<LightComponent>())
			{
				auto& lc = GetComponent<LightComponent>();
				if(HasComponent<TagComponent>())
					lc.Direction = glm::normalize(lc.Target - glm::vec3(transformation.Transform[3]));
				else
				{
					const glm::vec3 pos = transformation.Transform * glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
					lc.Direction = glm::normalize(lc.Target - pos);
				}
			}
		}

		Entity Child = { rel.FirstChild, m_Owner };
		for (size_t i = 0; i < rel.Childrens; i++)
		{
			Child.UpdateMatrices();
			const auto next = Child.GetComponent<RelationshipComponent>().Next;
			Child = { next, m_Owner };
		}

	}
}