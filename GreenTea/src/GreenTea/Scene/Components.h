#ifndef _COMPONENTS
#define _COMPONENTS

#include "GreenTea/Assets/AssetManager.h"

#include "GreenTea/GPU/Texture.h"

#include "ScriptableEntity.h"

#include <glm.hpp>


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
	struct ENGINE_API TransformComponent {
		glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };

		TransformComponent(void) = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
			: Position(position), Scale(scale), Rotation(rotation) {}

	};

	/**
	* @brief Transfomation Component
	* @details Transformation Component is use internally by the Engine
	*	for entities position, size and rotation in both 2D space and 3D potetionaly in the future
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

	struct ENGINE_API MeshComponent {

		Ref<Asset> Mesh = CreateRef<Asset>(nullptr, AssetType::INVALID);
		std::string Filepath;
		int32 MaterialIndex = 0;//Only used on editor

		MeshComponent(void) = default;
		MeshComponent(const MeshComponent&) = default;

	};

	struct ENGINE_API PerspectiveCameraComponent {
		glm::vec3 Target{ 0.0f, 0.0f, 0.0f };
		glm::vec3 UpVector{ 0.0f, 1.0f, 0.0f };
		float FoV = 60.0f;
		float Near = 0.1f;
		float Far = 1500.0f;

		PerspectiveCameraComponent(void) = default;
		PerspectiveCameraComponent(const PerspectiveCameraComponent&) = default;
		PerspectiveCameraComponent(const glm::vec3& target, float fov)
			: Target(target), FoV(fov) {}

		PerspectiveCameraComponent(const glm::vec3& target, glm::vec3 upVector, float fov)
			: Target(target), UpVector(upVector), FoV(fov) {}

		PerspectiveCameraComponent(const glm::vec3& target, const glm::vec3& upVector, float fov, float near, float far)
			: Target(target), UpVector(upVector), FoV(fov), Near(near), Far(far) {}

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

	enum class ENGINE_API LightType : char {
		Directional = 0x00,
		PointLight = 0x01,
		SpotLight = 0x02,
		AreaLight = 0x03
	};

	struct ENGINE_API LightComponent {
		glm::vec3 Target{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Direction{ 0.0f, -1.0f, 0.0f };//12 o'clock directional light
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Intensity = 120.0f;
		float Umbra = 60.0f;
		float Penumbra = 60.0f;
		float Near = 0.1f;
		float Far = 10.0f;
		float ShadowMapBias = 0.0002f;
		LightType Type = LightType::PointLight;
		
		LightComponent(void) = default;
		LightComponent(const LightComponent&) = default;
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
		glm::vec2 ShadowmapResolution {1024.0f, 1024.0f};
		float CamVelocity = 5.0f;
		ScenePropertiesComponent(void) = default;
		ScenePropertiesComponent(const ScenePropertiesComponent&) = default;
		
		ScenePropertiesComponent(const glm::vec2& shadowRes, float vel)
			: ShadowmapResolution(shadowRes), CamVelocity(vel) {}

	};

	struct ENGINE_API EnviromentComponent {
		Ref<Asset> Skybox = CreateRef<Asset>(nullptr, AssetType::INVALID);
		std::string SkyboxFilepath;

		EnviromentComponent(void) = default;
		EnviromentComponent(const EnviromentComponent&) = default;
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