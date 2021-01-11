#include "Scene.h"
#include "Entity.h"
#include "SceneSerialization.h"
#include "CollisionDispatcher.h"

#include "GreenTea/Core/DynamicLibLoader.h"
#include "GreenTea/Core/Math.h"

#include "GreenTea/Renderer/Renderer2D.h"

#include <box2d/box2d.h>

#include <gtc/constants.hpp>
#include <gtc/matrix_transform.hpp>
#include <fstream>

void CreateTransform2D(entt::registry& reg, entt::entity entity);
void DestroyRigidBody(entt::registry& reg, entt::entity entity);
void DestroyCircleCollider(entt::registry& reg, entt::entity entity);
void DestroyBoxCollider(entt::registry& reg, entt::entity entity);
void DestroyTransform2D(entt::registry& reg, entt::entity entity);

namespace GTE {

	static DynamicLibLoader GameLogic;
	static Scene* ActiveScene = nullptr;

	void Scene::Render(const glm::mat4* eyematrix)
	{

		if (eyematrix != nullptr) Renderer2D::BeginScene(*eyematrix);
		else
		{
			auto view = m_Registry.view<CameraComponent>();
			bool FoundCamera = false;
			for (auto entity : view)
			{
				auto& camComponent = view.get(entity);
				if (camComponent.Primary)
				{
					Renderer2D::BeginScene(camComponent);
					FoundCamera = true;
					break;
				}
			}
			if (!FoundCamera) return;
		}

		entt::insertion_sort algo;
		auto group = m_Registry.group<Renderable2DComponent>(entt::get<TransformationComponent>);
		group.sort([&](const entt::entity lhs, const entt::entity rhs)
			{
				const auto& transformationLeft = group.get<TransformationComponent>(lhs);
				const auto& transformationRight = group.get<TransformationComponent>(rhs);

				const glm::vec3 LeftPosition = transformationLeft.Transform[3];
				const glm::vec3 RightPosition = transformationRight.Transform[3];

				return LeftPosition.z < RightPosition.z;
			}, algo);

		for (auto enttID : group)
		{
			auto [renderable, transformation] = group.get<Renderable2DComponent, TransformationComponent>(enttID);
			if (!renderable.Filepath.empty() && renderable.Texture->Type != AssetType::TEXTURE)
				renderable.Texture = AssetManager::RequestTexture(renderable.Filepath.c_str());
			if (renderable.Texture->Type != AssetType::TEXTURE)
				Renderer2D::DrawQuad(transformation, (uint32)enttID, renderable.Color);
			else
			{
				TextureCoordinates coords = renderable.TextCoords;
				if (renderable.FlipX)
				{
					glm::vec2 temp = coords.BottomLeft;
					coords.BottomLeft = coords.BottomRight;
					coords.BottomRight = temp;

					temp = coords.TopLeft;
					coords.TopLeft = coords.TopRight;
					coords.TopRight = temp;
				}
				if (renderable.FlipY)
				{
					glm::vec2 temp = coords.BottomLeft;
					coords.BottomLeft = coords.TopLeft;
					coords.TopLeft = temp;

					temp = coords.BottomRight;
					coords.BottomRight = coords.TopRight;
					coords.TopRight = temp;
				}
				Renderer2D::DrawQuad(transformation, (GPU::Texture*)renderable.Texture->ActualAsset, coords, (uint32)enttID, renderable.Color);
			}
		}

		Renderer2D::EndScene();
	}

	void Scene::FixedUpdate(void)
	{
		auto& sceneProp = m_Registry.get<ScenePropertiesComponent>(m_Me);
		const float STEP = 1.0f / sceneProp.Rate;

		//Perform a Physics step
		sceneProp.World->Step(STEP, sceneProp.VelocityIterations, sceneProp.PositionIterations);


		{//Inform Engine about Changes cause by the Physics simulation
			auto view = m_Registry.view<RigidBody2DComponent>();
			auto group = m_Registry.group<RelationshipComponent, Transform2DComponent, TransformationComponent>();
			for (auto entity : view)
			{
				auto& rigidbody = view.get(entity);
				if (rigidbody.Body == nullptr || rigidbody.Type == BodyType::Static)
					continue;
				auto [rel, transform, transformation] = group.get<RelationshipComponent, Transform2DComponent, TransformationComponent>(entity);
				bool isWorld = false;
				if (rel.Parent == entt::null) isWorld = true;
				else if (group.find(rel.Parent) == group.end()) isWorld = true;
				if (isWorld)
				{
					transform.Position.x = rigidbody.Body->GetPosition().x;
					transform.Position.y = rigidbody.Body->GetPosition().y;
					transform.Rotation = glm::degrees(rigidbody.Body->GetAngle());
				}
				else
				{
					glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(rigidbody.Body->GetPosition().x, rigidbody.Body->GetPosition().y, transformation.Transform[3].z)) *
						glm::rotate(glm::mat4(1.0f), rigidbody.Body->GetAngle(), glm::vec3(0.0f, 0.0f, 1.0f));
					world = glm::inverse(group.get<TransformationComponent>(rel.Parent).Transform) * world;
					glm::vec3 Position, Rotation, Scale;
					Math::DecomposeTransform(world, Position, Rotation, Scale);
					transform.Position.x = Position.x;
					transform.Position.y = Position.y;
					transform.Rotation = glm::degrees(rigidbody.Body->GetAngle());
				}

				rigidbody.Velocity = glm::vec2(rigidbody.Body->GetLinearVelocity().x, rigidbody.Body->GetLinearVelocity().y);
				rigidbody.AngularVelocity = glm::degrees(rigidbody.Body->GetAngularVelocity());
			}
		}

		//Scripts Fixed Update
		auto scriptView = m_Registry.view<NativeScriptComponent>();
		scriptView.each([](auto& nScript) {
			if (nScript.State == ScriptState::Active)
				nScript.Instance->FixedUpdate();
			});

		UpdateMatrices();

		{//Inform Physics Engine about changes
			auto view = m_Registry.view<TransformationComponent>();
			auto group = m_Registry.group<RigidBody2DComponent>(entt::get<NativeScriptComponent>);
			for (auto entity : group)
			{
				auto& rigidbody = group.get<RigidBody2DComponent>(entity);
				if (rigidbody.Body == nullptr)
					continue;
				const auto& transformation = view.get(entity).Transform;
				glm::vec3 Position, Rotation, Scale;
				Math::DecomposeTransform(transformation, Position, Rotation, Scale);
				rigidbody.Body->SetTransform({ Position.x, Position.y }, Rotation.z);
				rigidbody.Body->SetLinearVelocity(b2Vec2{ rigidbody.Velocity.x, rigidbody.Velocity.y });
				rigidbody.Body->SetAngularVelocity(glm::radians(rigidbody.AngularVelocity));
			}
		}

	}

	void Scene::Update(float dt)
	{
		if (m_Registry.get<ScenePropertiesComponent>(m_Me).World == nullptr)
		{
			SetupWorld();
			auto view = m_Registry.view<NativeScriptComponent>();
			std::vector<entt::entity> bin;
			for (auto entity : view) {
				auto& nScript = view.get(entity);

				if (nScript.State == ScriptState::MustBeInitialized)
				{
					nScript.State = ScriptState::Active;
					nScript.Instance = GameLogic.CreateInstance<ScriptableEntity>(nScript.ClassName);
					if (nScript.Instance)
					{
						nScript.Instance->m_Entity = { entity , this };
						nScript.Instance->Start();
					}
					else
					{
						GTE_ERROR_LOG("Construction of object: ", nScript.ClassName, " failed!");
						bin.push_back(entity);
					}
				}

			}
			for (auto entity : bin)
			{
				Entity toDelete{ entity, this };
				toDelete.Destroy();
			}
			m_Accumulator = 0.0f;
			return;//Skip this frame
		}

		m_Accumulator += dt;

		const auto& sceneProp = m_Registry.get<ScenePropertiesComponent>(m_Me);
		const float STEP = 1.0f / (float)sceneProp.Rate;

		bool physics = false;
		while (m_Accumulator >= STEP)
		{
			physics = true;
			FixedUpdate();
			m_Accumulator -= STEP;
		}

		{
			//Update Transforms RigidBodies for next rendering (aka Move stuff on the screen)
			//	Physics has fixed time step so we need to "smooth" the movement between frames
			auto view = m_Registry.view<RigidBody2DComponent>();
			auto group = m_Registry.group<RelationshipComponent, Transform2DComponent, TransformationComponent>();
			for (auto enttID : view)
			{
				auto& rb = view.get(enttID);
				if (rb.Type == BodyType::Static)
					continue;
				if ((rb.Type == BodyType::Dynamic) && rb.Body != nullptr)
					{ if (!rb.Body->IsAwake()) continue; }

				float deltaTime = dt;
				if (physics && rb.Body != nullptr)
					deltaTime = m_Accumulator;

				auto [rel, transform, transformation] = group.get<RelationshipComponent, Transform2DComponent, TransformationComponent>(enttID);
				glm::vec3 Position, Rotation, Scale;
				Math::DecomposeTransform(transformation, Position, Rotation, Scale);
				Position += glm::vec3(rb.Velocity.x, rb.Velocity.y, 0.0f) * deltaTime;

			}
			/*
			auto group = m_Registry.view<RigidBody2DComponent, Transform2DComponent>();
			group.each([=](auto& rigidBody, auto& transform) {
				if (rigidBody.Type == BodyType::Static)
					return;
				if (rigidBody.Type == BodyType::Dynamic && rigidBody.Body != nullptr)
					{ if (!rigidBody.Body->IsAwake()) return; }

				float deltaTime = dt;
				if (physics && rigidBody.Body != nullptr)
					deltaTime = m_Accumulator;

				glm::vec3 Position, Rotation, Scale;

				Math::DecomposeTransform()
				glm::vec2 newPostion = glm::vec2(transform.Position.x, transform.Position.y) + rigidBody.Velocity * deltaTime;
				if (rigidBody.Type == BodyType::Dynamic)
					newPostion += 0.5f * sceneProp.Gravity * rigidBody.GravityFactor * deltaTime * deltaTime;

				transform.Position = glm::vec3(newPostion.x, newPostion.y, transform.Position.z);

				transform.Rotation += rigidBody.AngularVelocity * deltaTime;

				if (rigidBody.Type == BodyType::Dynamic)
					rigidBody.Velocity += sceneProp.Gravity * rigidBody.GravityFactor * deltaTime;

				});
			*/
		}

		//Scripts' Update
		std::vector<entt::entity> bin;
		auto view = m_Registry.view<NativeScriptComponent>();
		for (auto entity : view)
		{
			auto& nScript = view.get(entity);
			if (nScript.State == ScriptState::MustBeInitialized)
			{
				nScript.State = ScriptState::Active;
				nScript.Instance = GameLogic.CreateInstance<ScriptableEntity>(nScript.ClassName);
				if (nScript.Instance)
				{
					nScript.Instance->m_Entity = { entity , this };
					nScript.Instance->Start();
				}
				else
				{
					GTE_ERROR_LOG("Construction of object: ", nScript.ClassName, " failed!");
					bin.push_back(entity);
				}
			}
			else if (nScript.State == ScriptState::Active)
				nScript.Instance->Update(dt);
			else if (nScript.State == ScriptState::MustBeDestroyed)
			{
				nScript.Instance->Destroy();
				nScript.State = ScriptState::Inactive;
				delete nScript.Instance;
				bin.push_back(entity);
				//_registry.destroy(entity);
			}
		}

		//Clear Deleted entities
		for (auto entity : bin)
		{
			Entity toDelete = { entity, this };
			toDelete.Destroy();
		}

		UpdateMatrices();

	}

	//TODO: Change to use group in order to use his cashing capabilities in multiple threads
	void Scene::UpdateMatrices(void)
	{
		//Update all transformation Matrices
		auto group = m_Registry.group<RelationshipComponent, Transform2DComponent, TransformationComponent>();
		for (auto entity : group)
		{
			const auto& rel = group.get<RelationshipComponent>(entity);
			if (rel.Parent == entt::null)
			{
				Entity parent = { entity, this };
				parent.UpdateMatrices();
			}
		}
	}

	void Scene::Save(const char* filepath)
	{
		std::ofstream storage{ filepath, std::ofstream::out };

		{
			//Archives are flushing when getting out of scope
			cereal::JSONOutputArchive output{ storage };
			save(output, m_Registry);
		}

		storage.close();
	}

	void Scene::Load(const char* filepath)
	{
		m_Registry.clear();

		std::ifstream storage{ filepath, std::ifstream::in };
		cereal::JSONInputArchive input{ storage };

		load(input, m_Registry);
		storage.close();

		{//Renderable 2D Component's might need Texture Assets to be loaded
			auto view = m_Registry.view<Renderable2DComponent>();
			for (auto entityID : view)
			{
				auto& renderable = view.get(entityID);
				if (renderable.Filepath.empty()) continue;
				renderable.Texture = AssetManager::RequestTexture(renderable.Filepath.c_str());
			}
		}

		{//Load _me
			auto view = m_Registry.view<ScenePropertiesComponent>();
			for (auto entity : view) m_Me = entity;
		}

		UpdateMatrices();
	}

	void Scene::DestroyWorld()
	{
		auto& sceneProp = m_Registry.get<ScenePropertiesComponent>(m_Me);
		ENGINE_ASSERT(sceneProp.World != nullptr, "World already deleted!");
		delete sceneProp.World;
		sceneProp.World = nullptr;

		auto view = m_Registry.view<RigidBody2DComponent>();
		view.each([](auto& rigidBody) {rigidBody.Body = nullptr; });
	}

	void Scene::SetupWorld(void)
	{
		CollisionDispatcher::Get().SetContext(this);
		auto& sceneProp = m_Registry.get<ScenePropertiesComponent>(m_Me);
		ENGINE_ASSERT(sceneProp.World == nullptr, "World is already setup!");
		glm::vec2 g = sceneProp.Gravity;
		sceneProp.World = new b2World(b2Vec2(g.x, g.y));
		sceneProp.World->SetContactListener(&CollisionDispatcher::Get());

		b2BodyDef(*CreateBodyDef)(const RigidBody2DComponent&, const Transform2DComponent&) = [](const auto& rigidBody, const auto& transform)
			-> b2BodyDef
		{
			b2BodyDef bodyDef;
			bodyDef.position.Set(transform.Position.x, transform.Position.y);
			bodyDef.linearVelocity.Set(rigidBody.Velocity.x, rigidBody.Velocity.y);
			bodyDef.angle = glm::radians(transform.Rotation);
			bodyDef.angularVelocity = glm::radians(rigidBody.AngularVelocity);
			bodyDef.fixedRotation = rigidBody.FixedRotation;
			bodyDef.bullet = rigidBody.Bullet;
			switch (rigidBody.Type)
			{
			case BodyType::Dynamic:
				bodyDef.type = b2_dynamicBody;
				break;
			case BodyType::Kinematic:
				bodyDef.type = b2_kinematicBody;
				break;
			case BodyType::Static:
				bodyDef.type = b2_staticBody;
			default:
				break;
			}
			return bodyDef;
		};

		{//Setup Circle Colliders
			auto view = m_Registry.view<TransformationComponent>();
			auto group = m_Registry.group<CircleColliderComponent>(entt::get<RigidBody2DComponent>);
			for (auto entity : group)
			{
				auto [collider, rigidbody] = group.get<CircleColliderComponent, RigidBody2DComponent>(entity);
				const auto& transformation = view.get(entity);
				glm::vec3 Position, Rotation, Scale;
				Math::DecomposeTransform(transformation.Transform, Position, Rotation, Scale);
				b2BodyDef bodyDef = CreateBodyDef(rigidbody, { Position, glm::vec2(Scale.x, Scale.y), Rotation.z });
				rigidbody.Body = sceneProp.World->CreateBody(&bodyDef);
				b2CircleShape shape;
				shape.m_radius = collider.Radius;
				b2FixtureDef fixture;
				fixture.userData.pointer = static_cast<uintptr_t>(entity);
				fixture.shape = (b2Shape*)&shape;
				fixture.friction = collider.Friction;
				fixture.restitution = collider.Restitution;
				if (rigidbody.Type == BodyType::Dynamic)
					fixture.density = rigidbody.Mass / (glm::pi<float>() * collider.Radius * collider.Radius);
				collider.Fixture = rigidbody.Body->CreateFixture(&fixture);
			}
		}

		{//Setup Box Colliders
			auto group = m_Registry.group<RelationshipComponent, Transform2DComponent, TransformationComponent>();
			auto colliders = m_Registry.group<BoxColliderComponent>(entt::get<RigidBody2DComponent>);
			for (auto entity : colliders)
			{
				auto [collider, rigidbody] = colliders.get<BoxColliderComponent, RigidBody2DComponent>(entity);
				auto [rel, transform] = group.get<RelationshipComponent, Transform2DComponent>(entity);

				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), transform.Position) *
					glm::rotate(glm::mat4(1.0f), glm::radians(transform.Rotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
					glm::scale(glm::mat4(1.0f), glm::vec3(collider.Scale.x, collider.Scale.y, 1.0f));

				if (rel.Parent != entt::null)
				{
					if (m_Registry.has<TransformationComponent>(rel.Parent))
						transformation = group.get<TransformationComponent>(rel.Parent).Transform * transformation;
				}

				glm::vec3 Position, Rotation, Scale;
				Math::DecomposeTransform(transformation, Position, Rotation, Scale);
				b2BodyDef bodyDef = CreateBodyDef(rigidbody, { Position, { Scale.x, Scale.y }, Rotation.z });
				rigidbody.Body = sceneProp.World->CreateBody(&bodyDef);
				b2PolygonShape shape;
				shape.SetAsBox(Scale.x, Scale.y);
				b2FixtureDef fixture;
				fixture.userData.pointer = static_cast<uintptr_t>(entity);
				fixture.shape = (b2Shape*)&shape;
				fixture.friction = collider.Friction;
				fixture.restitution = collider.Restitution;
				if (rigidbody.Type == BodyType::Dynamic)
					fixture.density = rigidbody.Mass / (4 * collider.Scale.x, collider.Scale.y);
				collider.Fixture = rigidbody.Body->CreateFixture(&fixture);
			}
		}

	}

	void Scene::LoadGameLogic(const char* filepath)
	{
		if (filepath)
			GameLogic.Load(filepath);
		else
			GameLogic.Reload();
	}

	void Scene::UnloadGameLogic(void)
	{
		auto view = m_Registry.view<NativeScriptComponent>();
		for (auto entity : view)
		{
			auto& nScript = view.get(entity);
			if (nScript.State >= ScriptState::Active)
			{
				nScript.Instance->Destroy();
				delete nScript.Instance;
				nScript.State = ScriptState::MustBeInitialized;
			}
		}
		GameLogic.Unload();
	}

	Entity Scene::GetEditorCamera(void)
	{
		auto view = m_Registry.view<CameraComponent>(entt::exclude<TagComponent>);
		for (auto entityID : view)
			return Entity{ entityID, this };

		return Entity{ entt::null, nullptr };
	}

	Scene::Scene(void) //: transformationPool(_registry)
	{
		ActiveScene = this;
		//Setup scene Properties
		m_Me = m_Registry.create();
		auto& sceneProp = m_Registry.emplace<ScenePropertiesComponent>(m_Me);
		m_Registry.emplace<RelationshipComponent>(m_Me);

		m_Registry.on_construct<Transform2DComponent>().connect<&CreateTransform2D>();
		m_Registry.on_destroy<RigidBody2DComponent>().connect<&DestroyRigidBody>();
		m_Registry.on_destroy<CircleColliderComponent>().connect<&DestroyCircleCollider>();
		m_Registry.on_destroy<BoxColliderComponent>().connect<&DestroyBoxCollider>();
		m_Registry.on_destroy<Transform2DComponent>().connect<&DestroyTransform2D>();

		//Setup Editor's Camera
		auto EditorCamera = m_Registry.create();
		m_Registry.emplace<Transform2DComponent>(EditorCamera);
		m_Registry.emplace<CameraComponent>(EditorCamera);
		auto& ortho = m_Registry.emplace<OrthographicCameraComponent>(EditorCamera);
		ortho.VerticalBoundary = 10.0f;
		m_Registry.emplace<RelationshipComponent>(EditorCamera);
	}

	void Scene::TakeSnapshot(void)
	{
		if (m_Registry.get<ScenePropertiesComponent>(m_Me).World != nullptr) DestroyWorld();
		//Clear last snapshot
		m_TempStorage.clear();
		m_TempStorage.str(std::string());
		{
			//Archives are flushing when getting out of scope
			cereal::BinaryOutputArchive output{ m_TempStorage };
			entt::snapshot{ m_Registry }.entities(output).component<TagComponent,
				Transform2DComponent,
				Renderable2DComponent,
				OrthographicCameraComponent, CameraComponent,
				NativeScriptComponent,
				RigidBody2DComponent,
				CircleColliderComponent, BoxColliderComponent,
				ScenePropertiesComponent,
				RelationshipComponent>(output);
		}
	}

	void Scene::ReloadSnapshot(void)
	{
		m_Registry.clear();

		cereal::BinaryInputArchive input{ m_TempStorage };

		entt::snapshot_loader{ m_Registry }.entities(input).component<TagComponent,
			Transform2DComponent,
			Renderable2DComponent,
			OrthographicCameraComponent, CameraComponent,
			NativeScriptComponent,
			RigidBody2DComponent,
			CircleColliderComponent, BoxColliderComponent,
			ScenePropertiesComponent,
			RelationshipComponent>(input);

		{//Renderable 2D Component's might need Texture Assets to be loaded
			auto view = m_Registry.view<Renderable2DComponent>();
			for (auto entityID : view)
			{
				auto& renderable = view.get(entityID);
				if (renderable.Filepath.empty()) continue;
				renderable.Texture = AssetManager::RequestTexture(renderable.Filepath.c_str());
			}
		}

		{//Load me
			auto view = m_Registry.view<ScenePropertiesComponent>();
			for (auto entity : view) m_Me = entity;
		}

		UpdateMatrices();
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		auto ID = m_Registry.create();
		Entity entity = { ID, this };
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unamed Entity" : name;
		entity.AddComponent<RelationshipComponent>();
		return entity;
	}

	void Scene::onViewportResize(uint32 width, uint32 height)
	{
		float aspectRatio = (float)width / (float)height;
		auto view = m_Registry.view<CameraComponent, OrthographicCameraComponent>();
		for (auto entity : view)
		{
			auto [cam, ortho] = view.get<CameraComponent, OrthographicCameraComponent>(entity);
			cam.AspectRatio = aspectRatio;

			glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
			box *= glm::vec2(cam.AspectRatio, 1.0f);
			cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}
	}

	Scene::~Scene(void)
	{
		if (m_Registry.get<ScenePropertiesComponent>(m_Me).World)
			DestroyWorld();
		UnloadGameLogic();
	}

	Entity Scene::GetSceneEntity(void) { return { m_Me, this }; }

	std::vector<Entity> GetEntities(const std::string& Tag)
	{
		std::vector<Entity> entities;
		auto view = ActiveScene->m_Registry.view<TagComponent>();

		for (auto enttID : view)
		{
			const auto& TagComponent = view.get(enttID);
			if (Tag.compare(TagComponent.Tag) == 0)
				entities.push_back({ enttID, ActiveScene });
		}
		return entities;

	}

	
}


void CreateTransform2D(entt::registry& reg, entt::entity entity)
{
	auto& transform = reg.get<GTE::Transform2DComponent>(entity);
	
	glm::mat4 transformation = glm::translate(glm::mat4(1.0f), transform.Position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(transform.Rotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(transform.Scale.x, transform.Scale.y, 1.0f));

	reg.emplace<GTE::TransformationComponent>(entity, transformation);
}

void DestroyRigidBody(entt::registry& reg, entt::entity entity)
{
	auto view = reg.view<GTE::ScenePropertiesComponent>();
	b2World* world = view.get(view[0]).World;
	if (world == nullptr) return;
	b2Body* body = reg.get<GTE::RigidBody2DComponent>(entity).Body;
	if(body!=nullptr)
		world->DestroyBody(body);
}

void DestroyCircleCollider(entt::registry& reg, entt::entity entity)
{
	if (!reg.has<GTE::RigidBody2DComponent>(entity)) return;
	b2Body* body = reg.get<GTE::RigidBody2DComponent>(entity).Body;
	if (body == nullptr) return;
	b2Fixture* collider = reg.get<GTE::CircleColliderComponent>(entity).Fixture;
	body->DestroyFixture(collider);
}

void DestroyBoxCollider(entt::registry& reg, entt::entity entity)
{
	if (!reg.has<GTE::RigidBody2DComponent>(entity)) return;
	b2Body* body = reg.get<GTE::RigidBody2DComponent>(entity).Body;
	if (body == nullptr) return;
	b2Fixture* collider = reg.get<GTE::BoxColliderComponent>(entity).Fixture;
	body->DestroyFixture(collider);
}

void DestroyTransform2D(entt::registry& reg, entt::entity entity)
{
	if (reg.has<GTE::TransformationComponent>(entity))
		reg.remove<GTE::TransformationComponent>(entity);
}

