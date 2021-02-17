#include "Scene.h"
#include "Entity.h"
#include "SceneSerialization.h"

#include "GreenTea/Core/DynamicLibLoader.h"
#include "GreenTea/Core/Math.h"

#include "GreenTea/Renderer/Renderer.h"

#include <gtc/constants.hpp>
#include <gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

#include <fstream>

void CreateTransform(entt::registry& reg, entt::entity enttID);
void DestroyTransform(entt::registry& reg, entt::entity enttID);
void DestroyCamera(entt::registry& reg, entt::entity enttID);

namespace GTE {

	static DynamicLibLoader GameLogic;
	static Scene* ActiveScene = nullptr;

	void Scene::Render(const CameraComponent* cam, GPU::FrameBuffer* target, const glm::vec3& pos, const glm::vec3& dir)
	{
		SceneData data;
		data.Target = target;
		if (cam)
		{
			data.EyeMatrix = cam->EyeMatrix;
			data.ProjectionMatrix = cam->ProjectionMatrix;
			data.ViewMatrix = cam->ViewMatrix;

			data.CameraPos = pos;
			data.CameraDir = dir;
		}
		else
		{
			auto view = m_Registry.view<CameraComponent>();
			bool FoundCamera = false;
			for (auto enttID : view)
			{
				auto& camComponent = view.get(enttID);
				if (camComponent.Primary)
				{
					data.EyeMatrix = camComponent.EyeMatrix;
					data.ProjectionMatrix = camComponent.ProjectionMatrix;
					data.ViewMatrix = camComponent.ViewMatrix;

					data.CameraPos = m_Registry.get<TransformationComponent>(enttID).Transform[3];
					data.CameraDir = glm::normalize(m_Registry.get<PerspectiveCameraComponent>(enttID).Target - data.CameraPos);
					FoundCamera = true;
					break;
				}
			}
			if (!FoundCamera) return;
		}

		//entt::insertion_sort algo;
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformationComponent>);
		group.sort([&](const entt::entity lhs, const entt::entity rhs){
			const auto& transformationLeft = group.get<TransformationComponent>(lhs);
			const auto& transformationRight = group.get<TransformationComponent>(rhs);

			const glm::vec3 LeftPos = transformationLeft.Transform[3];
			const glm::vec3 RightPos = transformationRight.Transform[3];

			return Math::CompDistance(LeftPos, data.CameraPos) > Math::CompDistance(RightPos, data.CameraPos);
		});
		
		Renderer::BeginScene(data);

		{//Submit Lights
			auto view = m_Registry.view<LightComponent, TransformationComponent>();
			for (auto enttID : view)
			{
				glm::vec3 pos = m_Registry.get<TransformationComponent>(enttID).Transform[3];
				const auto& lc = view.get<LightComponent>(enttID);
				Renderer::SubmitLight(pos, lc);
			}
		}

		for (auto enttID : group)
		{
			auto [mc, transformation] = group.get<MeshComponent, TransformationComponent>(enttID);
			if (!mc.Filepath.empty() && (mc.Mesh->Type != AssetType::MESH))
				mc.Mesh = AssetManager::RequestMesh(mc.Filepath.c_str());
			if (mc.Mesh->Type == AssetType::MESH)
				Renderer::SubmitMesh(transformation, (GPU::Mesh*)mc.Mesh->ActualAsset, static_cast<uint32>(enttID));
		}

		GPU::CubicTexture* skybox = nullptr;
		{//Get skybox
			auto view = m_Registry.view<EnviromentComponent>();
			auto& env = view.get(*view.begin());
			if (!env.SkyboxFilepath.empty() && env.Skybox->Type != AssetType::TEXTURE)
				env.Skybox = AssetManager::RequestCubeMap(env.SkyboxFilepath.c_str());
			if (env.Skybox->Type == AssetType::TEXTURE)
				skybox = (GPU::CubicTexture*)env.Skybox->ActualAsset;
		}

		Renderer::EndScene(skybox);
	}


	void Scene::Update(float dt)
	{
		/*if (m_Registry.get<ScenePropertiesComponent>(m_Me).World == nullptr)
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
		}*/

		m_Accumulator += dt;

		/*{
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

				auto [rel, transform, transformation] = group.get<RelationshipComponent, TransformComponent, TransformationComponent>(enttID);
				glm::vec3 Position, Rotation, Scale;
				Math::DecomposeTransform(transformation, Position, Rotation, Scale);
				Position += glm::vec3(rb.Velocity.x, rb.Velocity.y, 0.0f) * deltaTime;

			}
		}*/

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

	//TODO: Use multiple thread to update all matrices in paralel
	void Scene::UpdateMatrices(void)
	{
		//Update all transformation Matrices
		auto group = m_Registry.group<RelationshipComponent, TransformComponent, TransformationComponent>();
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
				if (renderable.Filepath.empty())
					continue;
				renderable.Texture = AssetManager::RequestTexture(renderable.Filepath.c_str());
			}
		}

		{//Mesh Component might need to load Meshes
			auto view = m_Registry.view<MeshComponent>();
			for (auto enttID : view)
			{
				auto mc = view.get(enttID);
				if (mc.Filepath.empty())
					continue;
				mc.Mesh = AssetManager::RequestMesh(mc.Filepath.c_str());
			}
		}

		{//Load _me
			auto view = m_Registry.view<ScenePropertiesComponent>();
			for (auto entity : view) m_Me = entity;
		}

		UpdateMatrices();
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

	Scene::Scene(void)
	{
		ActiveScene = this;

		//Setup registry callbacks
		m_Registry.on_construct<TransformComponent>().connect<&CreateTransform>();
		m_Registry.on_destroy<TransformComponent>().connect<&DestroyTransform>();
		m_Registry.on_destroy<CameraComponent>().connect<&DestroyCamera>();

		//Setup scene Properties
		m_Me = m_Registry.create();
		auto& sceneProp = m_Registry.emplace<ScenePropertiesComponent>(m_Me);
		m_Registry.emplace<RelationshipComponent>(m_Me);
		EnviromentComponent env;
		env.SkyboxFilepath = "../Assets/Textures/Skybox/DefaultSkybox.png";
		m_Registry.emplace<EnviromentComponent>(m_Me, env);


		//Setup Editor's Camera
		auto EditorCamera = m_Registry.create();
		m_Registry.emplace<PerspectiveCameraComponent>(EditorCamera);
		m_Registry.emplace<CameraComponent>(EditorCamera);
		auto& tc = m_Registry.emplace<TransformComponent>(EditorCamera);
		tc.Position.z = -190.0f;
		tc.Position.y = 80.0f;

		m_Registry.emplace<RelationshipComponent>(EditorCamera);
		Entity{EditorCamera, this}.UpdateMatrices();
	}

	void Scene::TakeSnapshot(void)
	{
		//Clear last snapshot
		m_TempStorage.clear();
		m_TempStorage.str(std::string());
		{
			//Archives are flushing when getting out of scope
			cereal::BinaryOutputArchive output{ m_TempStorage };
			entt::snapshot{ m_Registry }.entities(output).component<TagComponent,
				TransformComponent,
				Renderable2DComponent, MeshComponent,
				PerspectiveCameraComponent, CameraComponent,
				LightComponent,
				NativeScriptComponent,
				ScenePropertiesComponent,
				RelationshipComponent>(output);
		}
	}

	void Scene::ReloadSnapshot(void)
	{
		m_Registry.clear();

		cereal::BinaryInputArchive input{ m_TempStorage };

		entt::snapshot_loader{ m_Registry }.entities(input).component<TagComponent,
			TransformComponent,
			Renderable2DComponent, MeshComponent,
			PerspectiveCameraComponent, CameraComponent,
			LightComponent,
			NativeScriptComponent,
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
		auto view = m_Registry.view<CameraComponent, PerspectiveCameraComponent>();
		for (auto entity : view)
		{
			auto [cam, persp] = view.get<CameraComponent, PerspectiveCameraComponent>(entity);
			cam.AspectRatio = aspectRatio;

			cam.ProjectionMatrix = glm::perspective(glm::radians(persp.FoV), cam.AspectRatio, persp.Near, persp.Far);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}
	}

	Scene::~Scene(void)
	{
		//if (m_Registry.get<ScenePropertiesComponent>(m_Me).World)
		//	DestroyWorld();
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


void CreateTransform(entt::registry& reg, entt::entity enttID)
{
	auto& transform = reg.get<GTE::TransformComponent>(enttID);
	
	glm::mat4 transformation = glm::translate(glm::mat4(1.0f), transform.Position) *
		glm::toMat4(glm::quat(glm::radians(transform.Rotation))) *
		glm::scale(glm::mat4(1.0f), transform.Scale);

	reg.emplace<GTE::TransformationComponent>(enttID, transformation);
}


void DestroyTransform(entt::registry& reg, entt::entity enttID)
{
	if (reg.has<GTE::TransformationComponent>(enttID))
		reg.remove<GTE::TransformationComponent>(enttID);
}

void DestroyCamera(entt::registry& reg, entt::entity enttID)
{
	if (reg.has<GTE::PerspectiveCameraComponent>(enttID))
		reg.remove<GTE::PerspectiveCameraComponent>(enttID);
}

