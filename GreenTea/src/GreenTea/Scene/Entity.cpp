#include "Entity.h"
#include "Components.h"
#include <gtc/matrix_transform.hpp>

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
		if (HasComponent<Transform2DComponent>())
		{
			auto& transform = newEntity.AddComponent<Transform2DComponent>();
			auto& ref = GetComponent<Transform2DComponent>();
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
		if (HasComponent<OrthographicCameraComponent>())
		{
			auto& camProp = newEntity.AddComponent<OrthographicCameraComponent>();
			auto& ref = GetComponent<OrthographicCameraComponent>();
			camProp.VerticalBoundary = ref.VerticalBoundary;
			camProp.ZoomLevel = ref.ZoomLevel;
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
		if (HasComponent<RigidBody2DComponent>())
		{
			auto& rigidbody = newEntity.AddComponent<RigidBody2DComponent>();
			auto& ref = GetComponent<RigidBody2DComponent>();
			rigidbody.AngularVelocity = ref.AngularVelocity;
			rigidbody.Bullet = ref.Bullet;
			rigidbody.FixedRotation = ref.FixedRotation;
			rigidbody.GravityFactor = ref.GravityFactor;
			rigidbody.Mass = ref.Mass;
			rigidbody.Type = ref.Type;
			rigidbody.Velocity = ref.Velocity;
		}
		if (HasComponent<BoxColliderComponent>())
		{
			auto& collider = newEntity.AddComponent<BoxColliderComponent>();
			auto& ref = GetComponent<BoxColliderComponent>();
			collider.Friction = ref.Friction;
			collider.Restitution = ref.Restitution;
			collider.Scale = ref.Scale;
		}
		if (HasComponent<CircleColliderComponent>())
		{
			auto& collider = newEntity.AddComponent<CircleColliderComponent>();
			auto& ref = GetComponent<CircleColliderComponent>();
			collider.Radius = ref.Radius;
			collider.Friction = ref.Friction;
			collider.Restitution = ref.Restitution;
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
			const auto& transform = GetComponent<Transform2DComponent>();
			auto& transformation = GetComponent<TransformationComponent>();

			transformation = glm::translate(glm::mat4(1.0f), transform.Position) *
				glm::rotate(glm::mat4(1.0f), glm::radians(transform.Rotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(transform.Scale.x, transform.Scale.y, 1.0f));

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
				camera.ViewMatrix = glm::inverse(transformation.Transform);
				camera.EyeMatrix = camera.ProjectionMatrix * camera.ViewMatrix;
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