#pragma once

#include <Engine/Assets/Asset.h>
#include <Engine/Assets/NativeScript.h>
#include <Engine/Assets/Animation.h>
#include <Engine/Audio/Source.h>
#include <Engine/Core/Ref.h>
#include <Engine/GPU/Texture.h>
#include <Engine/Renderer/ParticleSystem.h>

#include <glm.hpp>
#include <entt.hpp>

namespace gte {

	struct ENGINE_API IDComponent {
		uuid ID;

		IDComponent(void) = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const uuid& id)
			: ID(id) {}

		operator uuid& () { return ID; }
		operator const uuid& () const { return ID; }
	};

	/**
	* @brief Basically represents entity's name
	*/
	struct ENGINE_API TagComponent {
		std::string Tag;

		TagComponent(void) = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
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

	/**
	* @brief Position, Scale and Orientation for an object in 2D space
	* @details Always in local space
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
	* @brief Cached matrix entity's Transform
	* @details Because the values here store the global Transform of this object (The only reason isn't just caching the matrixes)
	*/
	struct ENGINE_API TransformationComponent {
		glm::mat4 Transform;

		TransformationComponent(void) = default;
		TransformationComponent(const TransformationComponent&) = default;
		TransformationComponent(const glm::mat4& transform) : Transform(transform) {}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	/**
	* @brief Basically a sprite's decscription
	*/
	struct ENGINE_API SpriteRendererComponent {
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Asset> Texture = CreateRef<Asset>();
		float TilingFactor = 1.0f;

		bool FlipX = false;
		bool FlipY = false;

		bool Visible = true;
		TextureCoordinates Coordinates;
		SpriteRendererComponent(void) = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};

	struct ENGINE_API CircleRendererComponent {
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		bool Visible = true;
		CircleRendererComponent(void) = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct ENGINE_API TextRendererComponent {
		std::string Text = "";
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Asset> Font = CreateRef<Asset>();
		uint32 Size = 32;

		bool Visible = true;

		TextRendererComponent(void) = default;
		TextRendererComponent(const TextRendererComponent&) = default;
	};

	struct ENGINE_API OrthographicCameraComponent {
		float ZoomLevel = 1.0f;
		float VerticalBoundary = 4.5f;

		OrthographicCameraComponent(void) = default;
		OrthographicCameraComponent(const OrthographicCameraComponent&) = default;
		OrthographicCameraComponent(float zoomLevel, float verticalBoundary)
			: ZoomLevel(zoomLevel), VerticalBoundary(verticalBoundary) {}

	};


	enum class ENGINE_API DistanceModel : byte {
		None = 0,
		Inverse,
		InverseClamp,
		Linear,
		LinearClamp,
		Exponent,
		ExponentClamp
	};

	struct ENGINE_API CameraComponent {
		float AspectRatio = 1.0f;

		bool Primary = false;
		bool FixedAspectRatio = false;

		glm::mat4 ProjectionMatrix{ 1.0f };
		glm::mat4 ViewMatrix{ 1.0f };
		glm::mat4 EyeMatrix{ 1.0f };

		float MasterVolume = 1.0f;
		DistanceModel Model = DistanceModel::None;

		CameraComponent(void) = default;
		CameraComponent(const CameraComponent&) = default;

		operator glm::mat4& (void) { return EyeMatrix; }
		operator const glm::mat4& (void) const { return EyeMatrix; }
	};

	enum class ENGINE_API ScriptState : byte {
		Inactive = 0x00,
		MustBeInitialized = 0x01,
		Active = 0x02,
		MustBeDestroyed = 0x03
	};

	//Forward Declaration(s)
	class ScriptableEntity;

	struct ENGINE_API NativeScriptComponent {
		ScriptableEntity* Instance = nullptr;
		ScriptState State = ScriptState::MustBeInitialized;
		Ref<Asset> ScriptAsset = CreateRef<Asset>();
		internal::NativeScript Description;

		NativeScriptComponent(void) = default;
		NativeScriptComponent(const NativeScriptComponent&) = default;

		NativeScriptComponent& operator=(const NativeScriptComponent&) = default;
	};

	enum class ENGINE_API BodyType : byte {
		Static = 0,
		Dynamic,
		Kinematic
	};

	struct ENGINE_API Rigidbody2DComponent {
		BodyType Type = BodyType::Static;
		void* Body = nullptr;

		glm::vec2 Velocity = { 0.0f, 0.0f };
		float AngularVelocity = 0.0f;
		float GravityFactor = 1.0f;

		bool FixedRotation = false;
		bool Bullet = false;

		Rigidbody2DComponent(void) = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct ENGINE_API Collider {
		void* Fixure = nullptr;

		glm::vec2 Offset = { 0.0f, 0.0f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;
		bool Sensor = false;
		Collider(void) = default;
		Collider(const Collider&) = default;
	};

	struct ENGINE_API BoxColliderComponent : public Collider {
		glm::vec2 Size = { 0.5f, 0.5f };

		BoxColliderComponent(void) = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
	};

	struct ENGINE_API CircleColliderComponent : public Collider {
		float Radius = 0.5f;

		CircleColliderComponent(void) = default;
		CircleColliderComponent(const CircleColliderComponent&) = default;
	};

	struct ENGINE_API Settings {
		glm::vec2 Gravity = { 0.0f, -9.8f };
		int32 Rate = 30;
		int32 VelocityIterations = 6;
		int32 PositionIterations = 2;

		glm::vec2 CameraVelocity = {9.0f, 9.0f};
		
		Settings(void) = default;
		Settings(const Settings&) = default;
	};

	struct ENGINE_API SpeakerComponent {
		audio::Source Source;
		Ref<Asset> AudioClip = CreateRef<Asset>();
		float Volume = 1.0f;
		float Pitch = 1.0f;
		float RollOffFactor = 1.0f;
		float RefDistance = 1.0f;
		float MaxDistance = 500.0f;
		bool Looping = false;
		bool PlayOnStart = false;

		SpeakerComponent(void) = default;
		SpeakerComponent(const SpeakerComponent&) = default;
	};

	struct ENGINE_API ParticleSystemComponent {
		internal::ParticleSystem* System = nullptr;
		internal::ParticleProps Props;
		bool PlayOnStart = false;
		
		ParticleSystemComponent(void) = default;
		ParticleSystemComponent& operator=(const ParticleSystemComponent&) = default;
	};

	struct ENGINE_API AnimationComponent {
		Ref<Asset> Animation = CreateRef<Asset>();
		internal::Animation Description;

		AnimationComponent(void) = default;
		AnimationComponent& operator=(const AnimationComponent&) = default;
	};

	template<typename ...Components>
	struct ComponentGroup {};

	using AllComponents = ComponentGroup<Transform2DComponent, SpriteRendererComponent, CircleRendererComponent, TextRendererComponent, CameraComponent, OrthographicCameraComponent, NativeScriptComponent, Rigidbody2DComponent, BoxColliderComponent, CircleColliderComponent, SpeakerComponent, ParticleSystemComponent, AnimationComponent>;
}