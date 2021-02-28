#ifndef _COMPONENTS
#define _COMPONENTS

#include "GreenTea/Assets/AssetManager.h"

#include "GreenTea/GPU/Texture.h"

#include "ScriptableEntity.h"

#include <glm.hpp>

#include <box2d/b2_body.h>

//TODO: Creation of Applications Properties Component?

namespace GTE {
	/**
	* @brief Tag Component
	* @details Tag Component is basically representing the entity's name
	*/
	struct ENGINE_API TagComponent {
		std::string Tag;

		TagComponent(void) = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	/**
	* @brief Transform 2D Component
	* @details Transform 2D components are used for the position, size
	*	and rotation of entities in the 2D space
	*/
	struct ENGINE_API Transform2DComponent {
		glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
		glm::vec2 Scale{ 1.0f, 1.0f };
		float Rotation = 0.0f;

		Transform2DComponent(void) = default;
		Transform2DComponent(const Transform2DComponent&) = default;
		Transform2DComponent(const glm::vec3& position, const glm::vec2& scale, float rotation)
			: Position(position), Scale(scale), Rotation(rotation) {}

	};

	/**
	* @brief Transfomation Component
	* @details Transformation Component is use internally by the Engine
	*	for entities position, size and rotation in both 2D space (and 3D potetionaly in the future)
	*/
	struct ENGINE_API TransformationComponent {
		glm::mat4 Transform;

		TransformationComponent(void) = default;
		TransformationComponent(const TransformationComponent&) = default;
		TransformationComponent(glm::mat4 transform) : Transform(transform) {}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	/**
	* @brief Renderable 2D Component
	* @details Renderable components are used in 2D rendering
	*	the are basically textured sprites
	*/
	struct ENGINE_API Renderable2DComponent {
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Asset> Texture = CreateRef<Asset>(nullptr, AssetType::INVALID);
		std::string Filepath;

		bool FlipX = false;
		bool FlipY = false;

		TextureCoordinates TextCoords;

		Renderable2DComponent(void) = default;
		Renderable2DComponent(const Renderable2DComponent&) = default;
		Renderable2DComponent(const glm::vec4 color) : Color(color) {}

	};

	/**
	* @brief Enumaration for all availiable RigidBody types
	*/
	enum class ENGINE_API BodyType : char {
		Static = 0x00,
		Dynamic = 0x01,
		Kinematic = 0x02
	};

	
	struct ENGINE_API RigidBody2DComponent {
		b2Body* Body = nullptr;
		BodyType Type = BodyType::Static;

		glm::vec2 Velocity = { 0.0f, 0.0f };

		float AngularVelocity = 0.0f;
		float Mass = 1.0f;
		float GravityFactor = 1.0f;

		bool FixedRotation = false;
		bool Bullet = false;

		RigidBody2DComponent(void) = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;
		RigidBody2DComponent(const glm::vec2& velocity, float angularVelocity = 0.0f, float mass = 1.0f, float gravityFactor = 1.0f)
			: Velocity(velocity), AngularVelocity(angularVelocity), Mass(mass), GravityFactor(gravityFactor) {}

		RigidBody2DComponent(const glm::vec2& velocity, BodyType type, float angularVelocity = 0.0f, float mass = 1.0f, float gravityFactor = 1.0f)
			: Type(type), Velocity(velocity), AngularVelocity(angularVelocity), Mass(mass), GravityFactor(gravityFactor) {}

	};

	struct ENGINE_API Collider {
		b2Fixture* Fixture = nullptr;
		float Friction = 0.2f;
		float Restitution = 0.0f;

		Collider(void) = default;
		Collider(const Collider&) = default;
		Collider(float friction, float restitution)
			: Friction(friction), Restitution(restitution) {}

	};

	struct ENGINE_API CircleColliderComponent : public Collider{
		float Radius = 1.0f;

		CircleColliderComponent(void) = default;
		CircleColliderComponent(const CircleColliderComponent&) = default;
		CircleColliderComponent(float radius, float friction, float restitution)
			: Collider(friction, restitution), Radius(radius) {}

	};

	struct ENGINE_API BoxColliderComponent : public Collider {
		glm::vec2 Scale = { 1.0f, 1.0f };

		BoxColliderComponent(void) = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
		BoxColliderComponent(const glm::vec2 scale, float friction, float restitution)
			: Collider(friction, restitution), Scale(scale) {}

	};

	struct ENGINE_API OrthographicCameraComponent {
		float ZoomLevel = 1.0f;
		float VerticalBoundary = 1.0f;

		OrthographicCameraComponent(void) = default;
		OrthographicCameraComponent(const OrthographicCameraComponent&) = default;
		OrthographicCameraComponent(float zoomLevel, float verticalBoundary)
			: ZoomLevel(zoomLevel), VerticalBoundary(verticalBoundary) {}

	};

	struct ENGINE_API CameraComponent {
		float AspectRatio = 1.0f;

		bool Primary = false;
		bool FixedAspectRatio = false;

		glm::mat4 ProjectionMatrix{ 1.0f };
		glm::mat4 ViewMatrix{ 1.0f };
		glm::mat4 EyeMatrix{ 1.0f };

		CameraComponent(void) = default;
		CameraComponent(const CameraComponent&) = default;

		operator glm::mat4& (void) { return EyeMatrix; }
		operator const glm::mat4& (void) const { return EyeMatrix; }
	};

	enum class ENGINE_API ScriptState : char {
		Inactive = 0x00,
		MustBeInitialized = 0x01,
		Active = 0x02,
		MustBeDestroyed = 0x03
	};

	struct ENGINE_API NativeScriptComponent {
		ScriptableEntity* Instance = nullptr;
		ScriptState State = ScriptState::Inactive;
		std::string ClassName = std::string();

		NativeScriptComponent(void) = default;
		NativeScriptComponent(const NativeScriptComponent&) = default;
		NativeScriptComponent(const std::string& className) : ClassName(className) {}
		NativeScriptComponent(const std::string& className, ScriptState state)
			: ClassName(className), State(state) {}

	};

	/**
	* @brief Component for holding Scene's Properties
	* @warning This component is meant to be used internally by the engine
	*/
	struct ENGINE_API ScenePropertiesComponent {
		b2World* World = nullptr;

		/**
		* @brief Gravity Acceleration
		*/
		glm::vec2 Gravity = { 0.0f, -10.0f };
		/**
		* @brief Physics fixed-update rate
		* @details How many fixed-updates and physics calculations will be happening each second
		*/
		int32 Rate = 30;

		int32 VelocityIterations = 6;
		int32 PositionIterations = 2;

		glm::vec2 CamVelocity {1.0f, 1.0f};

		ScenePropertiesComponent(void) = default;
		ScenePropertiesComponent(const ScenePropertiesComponent&) = default;
		ScenePropertiesComponent(const glm::vec2 g, int32 rate, int32 velIterations = 6, int32 posIterations = 2)
			: Gravity(g), Rate(rate), VelocityIterations(velIterations), PositionIterations(posIterations) {}

	};

	/**
	* @brief Component for representing Relationship between entities
	* @sa You can read more about the implementation here:
	*	https://skypjack.github.io/2019-06-25-ecs-baf-part-4/
	*/
	struct ENGINE_API RelationshipComponent {
		/**
		* @brief Number of Childrens entities
		*/
		size_t Childrens = 0;
		/**
		* @brief First Children
		*/
		entt::entity FirstChild{ entt::null };
		/**
		* @brief Previous sibling
		*/
		entt::entity Previous{ entt::null };
		/**
		* @brief Next sibling
		*/
		entt::entity Next{ entt::null };
		/**
		* @brief Parent Entity
		*/
		entt::entity Parent{ entt::null };

		RelationshipComponent(void) = default;
		RelationshipComponent(const RelationshipComponent&) = default;

	};

}

#endif