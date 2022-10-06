#include "Scene.h"
#include "Components.h"
#include "CollisionDispatcher.h"
#include "Entity.h"

#include <Engine/Core/Context.h>
#include <Engine/Core/Math.h>
#include <Engine/NativeScripting/ScriptableEntity.h>
#include <Engine/Renderer/Renderer2D.h>

#include <gtc/matrix_transform.hpp>
// Box2D
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

static void CreateTransform2D(entt::registry& reg, entt::entity entityID);
static void DestroyTransform2D(entt::registry& reg, entt::entity entityID);

static void CreateCamera(entt::registry& reg, entt::entity entityID);
static void DestroyCamera(entt::registry& reg, entt::entity entityID);
static b2BodyDef CreateBody(const gte::Rigidbody2DComponent& rb, const glm::vec3& pos, float angle);
static bool HasParentTransform(const gte::RelationshipComponent& relc, const entt::registry& reg, glm::mat4& pTransform);

namespace gte {

	void Scene::Update(float dt)
	{
		const float STEP = 1.0f / FindEntityWithUUID({}).GetComponent<Settings>().Rate;
		mAccumulator += dt;
		bool physics = false;
		while (mAccumulator >= STEP)
		{
			physics = true;
			FixedUpdate();
			mAccumulator -= STEP;
		}

		Movement(dt, physics);

		//Handle Scripting logic
		std::vector<entt::entity> bin;
		auto scripts = mReg.view<NativeScriptComponent>();
		for (auto&& [entityID, nc] : scripts.each())
		{
			if (nc.State == ScriptState::MustBeInitialized)
			{
				auto name = nc.Description.GetName();
				std::replace(name.begin(), name.end(), ' ', '_');
				nc.Instance = internal::GetContext()->DynamicLoader.CreateInstance<ScriptableEntity>(name);
				internal::GetContext()->ScriptEngine->Instantiate(nc.Instance, nc.Description);
				nc.Instance->mEntity = { entityID, this };
				nc.Instance->Start();
				nc.State = ScriptState::Active;
			}
			else if (nc.State == ScriptState::Active)
				nc.Instance->Update(dt);
			else if (nc.State == ScriptState::MustBeDestroyed)
			{
				nc.Instance->Destroy();
				delete nc.Instance;
				nc.State = ScriptState::Inactive;
				bin.push_back(entityID);
			}
		}
		for (auto entityID : bin)
			DestroyEntity({ entityID, this });

		UpdateMatrices();

		//Find primary camera for rendering
		bool found = false;
		glm::mat4 eyeMatrix;
		auto cameras = mReg.view<CameraComponent>();
		for (auto&& [entityID, cam] : cameras.each())
		{
			if (cam.Primary)
			{
				eyeMatrix = cam;
				found = true;
				break;
			}
		}
		
		if (found)
			Render(eyeMatrix);
	}

	void Scene::UpdateEditor()
	{
		auto view = mReg.view<CameraComponent>(entt::exclude<TagComponent>);
		glm::mat4 eyeMatrix;
		for (auto entityID : view)//Only one should be inside
			eyeMatrix = view.get<CameraComponent>(entityID).EyeMatrix;
		Render(eyeMatrix);
	}

	void Scene::Render(const glm::mat4& eyematrix)
	{
		entt::insertion_sort algo;
		mReg.sort<TransformationComponent>([](const auto& lhs, const auto& rhs) { return lhs.Transform[3].z < rhs.Transform[3].z; }, algo);
		Renderer2D::BeginScene(eyematrix);
		auto view = mReg.view<TransformationComponent>();
		for (auto&& [entityID, tc] : view.each())
		{
			if (auto* sprite = mReg.try_get<SpriteRendererComponent>(entityID))
			{
				sprite->Texture = internal::GetContext()->AssetManager.RequestAsset(sprite->Texture->ID);
				if (sprite->Texture->Type == AssetType::INVALID)
					Renderer2D::DrawQuad(tc.Transform, (uint32)entityID, sprite->Color);
				else if (sprite->Texture->Type == AssetType::TEXTURE)
				{
					TextureCoordinates coords = sprite->Coordinates;
					if (sprite->FlipX)
					{
						glm::vec2 temp = coords.BottomLeft;
						coords.BottomLeft = coords.BottomRight;
						coords.BottomRight = temp;

						temp = coords.TopLeft;
						coords.TopLeft = coords.TopRight;
						coords.TopRight = temp;
					}
					if (sprite->FlipY)
					{
						glm::vec2 temp = coords.TopLeft;
						coords.TopLeft = coords.BottomLeft;
						coords.BottomLeft = temp;

						temp = coords.TopRight;
						coords.TopRight = coords.BottomRight;
						coords.BottomRight = temp;
					}
					Renderer2D::DrawQuad(tc.Transform, (GPU::Texture*)sprite->Texture->Data, coords, (uint32)entityID, sprite->Color, sprite->TilingFactor);
				}
			}
			else if(auto* circle = mReg.try_get<CircleRendererComponent>(entityID))
				Renderer2D::DrawCircle(tc.Transform, circle->Color, (uint32)entityID, circle->Thickness, circle->Fade);
		}
		Renderer2D::EndScene();
	}

	Entity Scene::CreateEntity(const std::string& name) { return CreateEntityWithUUID(uuid::Create(), name); }

	Entity Scene::CreateEntityWithUUID(const uuid& id, const std::string& name)
	{
		auto entityID = mReg.create();
		const auto tag = name.empty() ? "Unnamed Enity" : name;
		mReg.emplace<IDComponent>(entityID, id);
		mReg.emplace<TagComponent>(entityID, tag);
		mReg.emplace<RelationshipComponent>(entityID);
		return { entityID, this };
	}

	Entity Scene::CreateChildEntity(Entity parent)
	{
		Entity entity = CreateEntity();
		auto& prel = parent.GetComponent<RelationshipComponent>();
		auto nextID = prel.FirstChild;
		prel.FirstChild = entity;
		prel.Childrens++;

		auto& relc = entity.GetComponent<RelationshipComponent>();
		relc.Parent = parent;
		relc.Next = nextID;
		if (nextID != entt::null)
			mReg.get<RelationshipComponent>(nextID).Previous = entity;
		return entity;
		/*mReg.emplace<IDComponent>(entityID, uuid::Create());
		auto& tagc = mReg.emplace<TagComponent>(entityID, "Unnamed Entity");
		auto& relc = mReg.emplace<RelationshipComponent>(entityID);
		if (proto)
			relc = proto.GetComponent<RelationshipComponent>();
		relc.Parent = (entt::entity)parent;
		relc.Next = nextID;

		if(nextID != entt::null)
			mReg.get<RelationshipComponent>(nextID).Previous = entityID;

		auto child = relc.FirstChild;
		for (size_t i = 0; i < relc.Childrens; i++)
		{
			auto& childRel = mReg.get<RelationshipComponent>(child);
			childRel.Parent = entityID;
			child = childRel.Next;
		}

		if (proto)//Copy Components
			CopyComponents(proto, { entityID, this });
		
		return { entityID, this };
		*/
	}

	void Scene::MoveEntity(Entity parent, Entity toMove)
	{
		auto& prel = parent.GetComponent<RelationshipComponent>();
		auto nextID = prel.FirstChild;
		prel.FirstChild = toMove;
		prel.Childrens++;

		auto& relc = toMove.GetComponent<RelationshipComponent>();
		Entity oldParent = { relc.Parent, this };
		if (oldParent)
		{
			auto& oldprel = oldParent.GetComponent<RelationshipComponent>();
			if (oldprel.FirstChild == toMove)
				oldprel.FirstChild = relc.Next;
			else if (oldprel.Childrens > 1)
			{
				mReg.get<RelationshipComponent>(relc.Previous).Next = relc.Next;
				if (relc.Next != entt::null)
					mReg.get<RelationshipComponent>(relc.Next).Previous = relc.Previous;
			}
			oldprel.Childrens--;
		}

		if (relc.Previous != entt::null)
			mReg.get<RelationshipComponent>(relc.Previous).Next = relc.Next;
		relc.Next = nextID;
		relc.Previous = entt::null;
		relc.Parent = parent;
		if (nextID != entt::null)
			mReg.get<RelationshipComponent>(nextID).Previous = toMove;

		UpdateTransform(toMove);
	}

	Entity Scene::Clone(Entity toClone, bool recursive)
	{
		Entity entity = CreateEntity();
		CopyComponents(toClone, entity);

		const auto& relationship = toClone.GetComponent<RelationshipComponent>();
		auto& rel = entity.GetComponent<RelationshipComponent>();
		rel.Childrens = relationship.Childrens;
		Entity parent = { relationship.Parent, this };
		if (parent && !recursive)
		{
			rel.Parent = relationship.Parent;
			auto& prel = parent.GetComponent<RelationshipComponent>();
			prel.Childrens++;
			rel.Next = prel.FirstChild;
			prel.FirstChild = entity;
			mReg.get<RelationshipComponent>(rel.Next).Previous = entity;
		}

		Entity currChild = { relationship.FirstChild, this };
		Entity oldChild = {};
		if (currChild)
		{
			Entity child = Clone(currChild, true);
			auto& crel = child.GetComponent<RelationshipComponent>();
			crel.Parent = entity;
			rel.FirstChild = child;
			oldChild = child;
		}
		for (size_t i = 1; i < relationship.Childrens; i++)
		{
			Entity prevChild = currChild;
			const auto& prel = prevChild.GetComponent<RelationshipComponent>();
			currChild = { prel.Next, this };
			Entity child = Clone(currChild, true);
			auto& crel = child.GetComponent<RelationshipComponent>();
			crel.Parent = entity;
			crel.Previous = oldChild;
			mReg.get<RelationshipComponent>(oldChild).Next = child;
			oldChild = child;
		}

		UpdateTransform(entity);
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		//We need to start from the bottom up
		auto& relc = entity.GetComponent<RelationshipComponent>();
		const size_t childrens = relc.Childrens;
		if (childrens > 0)//Call destroy on children recursively
		{
			auto curr = relc.FirstChild;
			entt::entity next = mReg.get<RelationshipComponent>(curr).Next;
			for (size_t i = 0; i < childrens - 1; i++)
			{
				DestroyEntity({ curr, this });
				curr = next;
				next = mReg.get<RelationshipComponent>(curr).Next;
			}
			DestroyEntity({ curr, this });
		}

		//Now need to inform neighbours & parent
		if (relc.Previous != entt::null)
			mReg.get<RelationshipComponent>(relc.Previous).Next = relc.Next;
		if (relc.Next != entt::null)
			mReg.get<RelationshipComponent>(relc.Next).Previous = relc.Previous;
		if (relc.Parent != entt::null)
		{
			auto& prel = mReg.get<RelationshipComponent>(relc.Parent);
			prel.Childrens--;
			if (relc.Previous == entt::null)
				prel.FirstChild = relc.Next;
		}
		mReg.destroy(entity);
	}

	[[nodiscard]] Entity Scene::FindEntityWithUUID(const uuid& id)
	{
		auto view = mReg.view<IDComponent>();
		for (auto entityID : view)
		{
			const uuid& candidate = view.get<IDComponent>(entityID).ID;
			if (candidate == id)
				return { entityID, this };
		}
		return {};
	}

	void Scene::UpdateTransform(Entity entity)
	{
		const auto& tc = entity.GetComponent<Transform2DComponent>();
		auto& transform = entity.GetComponent<TransformationComponent>();
		transform.Transform = glm::translate(glm::mat4(1.0f), tc.Position) * glm::rotate(glm::mat4(1.0f), glm::radians(tc.Rotation), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { tc.Scale.x, tc.Scale.y, 1.0f });

		if (entity.HasComponent<RelationshipComponent>())
		{
			const auto& rel = entity.GetComponent<RelationshipComponent>();
			glm::mat4 pTransform;
			if (HasParentTransform(rel, mReg, pTransform))
				transform.Transform = pTransform * transform.Transform;
			
			Entity child = { rel.FirstChild, this };
			for (size_t i = 0; i < rel.Childrens; i++)
			{
				if(child.HasComponent<Transform2DComponent>())
					UpdateTransform(child);
				auto next = child.GetComponent<RelationshipComponent>().Next;
				child = { next, this };
			}
		}

		if (entity.HasComponent<CameraComponent>())
		{
			const auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
			auto& cam = entity.GetComponent<CameraComponent>();
			cam.ViewMatrix = glm::inverse(transform.Transform);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}
	}

	Scene::Scene(void)
	{
		mReg.on_construct<Transform2DComponent>().connect<&CreateTransform2D>();
		mReg.on_destroy<Transform2DComponent>().connect<&DestroyTransform2D>();
		mReg.on_construct<CameraComponent>().connect<&CreateCamera>();
		mReg.on_destroy<CameraComponent>().connect<&DestroyCamera>();

		auto me = mReg.create();
		mReg.emplace<IDComponent>(me);
		mReg.emplace<Transform2DComponent>(me);
		mReg.emplace<CameraComponent>(me);
		mReg.emplace<Settings>(me);
	}

	void Scene::OnViewportResize(uint32 width, uint32 height)
	{
		const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		auto view = mReg.view<CameraComponent>();
		for (auto entityID : view)
		{
			auto& cam = view.get<CameraComponent>(entityID);
			if (!cam.FixedAspectRatio)
			{
				cam.AspectRatio = aspectRatio;
				auto& ortho = mReg.get<OrthographicCameraComponent>(entityID);
				glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
				box *= glm::vec2(cam.AspectRatio, 1.0f);
				cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
				cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
			}
		}
	}

	void Scene::UpdateMatrices(void)
	{
		//TODO(Vasilis): Could use a thread pool to make this run in pararel
		auto view = mReg.view<RelationshipComponent, Transform2DComponent>();
		for (auto entityID: view)
		{
			const auto& rel = view.get<RelationshipComponent>(entityID);
			if (HasParentTransform(rel, mReg, glm::mat4()))
				continue;
			UpdateTransform({ entityID, this });
		}

		Entity me = FindEntityWithUUID({});//Special Entity for scene stuff
		UpdateTransform(me);
	}

	void Scene::CopyComponents(Entity source, Entity destination)
	{
		const auto& tag = source.GetComponent<TagComponent>().Tag;
		destination.GetComponent<TagComponent>().Tag = tag;

		if (source.HasComponent<Transform2DComponent>())
		{
			const auto& tc = source.GetComponent<Transform2DComponent>();
			destination.AddComponent<Transform2DComponent>(tc);

			UpdateTransform(destination);
		}

		if (source.HasComponent<SpriteRendererComponent>())
		{
			const auto& sprite = source.GetComponent<SpriteRendererComponent>();
			destination.AddComponent<SpriteRendererComponent>(sprite);
		}

		if (source.HasComponent<CircleRendererComponent>())
		{
			const auto& circle = source.GetComponent<CircleRendererComponent>();
			destination.AddComponent<CircleRendererComponent>(circle);
		}

		if (source.HasComponent<CameraComponent>())
		{
			const auto& cam = source.GetComponent<CameraComponent>();
			destination.AddComponent<CameraComponent>(cam);

			const auto& ortho = source.GetComponent<OrthographicCameraComponent>();
			auto& destination_ortho = destination.GetComponent<OrthographicCameraComponent>();
			destination_ortho = ortho;
		}

		if (source.HasComponent<NativeScriptComponent>())
		{
			const auto& nc = source.GetComponent<NativeScriptComponent>();
			destination.AddComponent<NativeScriptComponent>(nc);
		}

		if (source.HasComponent<Rigidbody2DComponent>())
		{
			const auto& rb = source.GetComponent<Rigidbody2DComponent>();
			destination.AddComponent<Rigidbody2DComponent>(rb);
		}

		if (source.HasComponent<BoxColliderComponent>())
		{
			const auto& bc = source.GetComponent<BoxColliderComponent>();
			destination.AddComponent<BoxColliderComponent>(bc);
		}

		if (source.HasComponent<CircleColliderComponent>())
		{
			const auto& cc = source.GetComponent<CircleColliderComponent>();
			destination.AddComponent<CircleColliderComponent>(cc);
		}
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<uuid, entt::entity>& enttMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				uuid id = src.get<IDComponent>(srcEntity).ID;
				if (!id.IsValid())
					continue;
				entt::entity dstEntity = enttMap.at(id);

				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<uuid, entt::entity>& enttMap) { CopyComponent<Component...>(dst, src, enttMap); }

	[[nodiscard]] Scene* Scene::Copy(Scene* other)
	{
		Scene* newScene = new Scene();
	
		auto& dstReg = newScene->mReg;
		auto& srcReg = other->mReg;
		
		{//Special entity for scene stuff
			auto dstEntityID = (entt::entity)newScene->FindEntityWithUUID({});
			auto srcEntityID = (entt::entity)other->FindEntityWithUUID({});
			dstReg.emplace_or_replace<Transform2DComponent>(dstEntityID, srcReg.get<Transform2DComponent>(srcEntityID));
			dstReg.emplace_or_replace<CameraComponent>(dstEntityID, srcReg.get<CameraComponent>(srcEntityID));
			dstReg.emplace_or_replace<OrthographicCameraComponent>(dstEntityID, srcReg.get<OrthographicCameraComponent>(srcEntityID));
			dstReg.emplace_or_replace<Settings>(dstEntityID, srcReg.get<Settings>(srcEntityID));
		}

		std::unordered_map<uuid, entt::entity> enttMap;
		{//Create entities
			auto view = srcReg.view<IDComponent, TagComponent>();
			for (auto&& [entityID, id, tag] : view.each())
			{
				entt::entity entity = newScene->CreateEntityWithUUID(id, tag.Tag);
				enttMap.insert({ id, entity });
				//entt ids' won't be valid since they are pointing source registry but they will be patch afterwards
				dstReg.emplace_or_replace<RelationshipComponent>(entity, srcReg.get<RelationshipComponent>(entityID));
			}
		}

		{//Patch relationships
			auto view = dstReg.view<RelationshipComponent>();
			for (auto&& [entityID, relc] : view.each())
			{
				if (relc.Parent != entt::null) relc.Parent = enttMap[srcReg.get<IDComponent>(relc.Parent)];
				if (relc.FirstChild != entt::null) relc.FirstChild = enttMap[srcReg.get<IDComponent>(relc.FirstChild)];
				if (relc.Next != entt::null) relc.Next = enttMap[srcReg.get<IDComponent>(relc.Next)];
				if (relc.Previous != entt::null) relc.Previous = enttMap[srcReg.get<IDComponent>(relc.Previous)];
			}
		}

		CopyComponent(AllComponents{}, dstReg, srcReg, enttMap);
		newScene->UpdateMatrices();
		return newScene;
	}

	void Scene::OnStart(void)
	{
		std::vector<entt::entity> bin;
		auto scripts = mReg.view<NativeScriptComponent>();
		for (auto&& [entityID, nc] : scripts.each())
		{
			if (nc.State == ScriptState::MustBeInitialized)
			{
				auto name = nc.Description.GetName();
				std::replace(name.begin(), name.end(), ' ', '_');
				nc.Instance = internal::GetContext()->DynamicLoader.CreateInstance<ScriptableEntity>(name);
				if (nc.Instance)
				{
					internal::GetContext()->ScriptEngine->Instantiate(nc.Instance, nc.Description);
					nc.Instance->mEntity = { entityID, this };
					nc.Instance->Start();
					nc.State = ScriptState::Active;
				}
				else
				{
					GTE_ERROR_LOG("Couldn't create instance of object: ", name);
					bin.push_back(entityID);
				}
			}
		}
		for (auto entityID : bin)
			DestroyEntity({ entityID, this });

		auto cameras = mReg.view<OrthographicCameraComponent, CameraComponent>();
		for (auto&& [entityID, ortho, cam] : cameras.each())
		{
			glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
			box *= glm::vec2(cam.AspectRatio, 1.0f);
			cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}

		UpdateMatrices();
		OnPhysicsStart();
	}

	void Scene::OnStop(void)
	{
		OnPhysicsStop();

		auto scripts = mReg.view<NativeScriptComponent>();
		for (auto&& [entityID, nc] : scripts.each())
		{
			if (!nc.Instance)
				continue;
			nc.Instance->Destroy();
			delete nc.Instance;
		}
	}

	void Scene::FixedUpdate(void)
	{
		Entity me = FindEntityWithUUID({});
		auto& settings = me.GetComponent<Settings>();
		const float STEP = 1.0f / settings.Rate;

		mPhysicsWorld->Step(STEP, settings.VelocityIterations, settings.PositionIterations);
		
		{// Inform game engine about changes by physics world
			auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
			for (auto&& [entityID, cc, rb, tc] : circles.each())
			{
				if (rb.Type == BodyType::Static)
					continue;
				InformEngine(entityID, rb, tc);
			}
			auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
			for (auto&& [entityID, bc, rb, tc] : boxes.each())
			{
				if (rb.Type == BodyType::Static)
					continue;
				InformEngine(entityID, rb, tc);
			}
		}
		//Call FixedUpdate() on scripts
		auto scripts = mReg.view<NativeScriptComponent>();
		scripts.each([](auto& script) {
			if (script.State == ScriptState::Active)
				script.Instance->FixedUpdate();
		});
		UpdateMatrices();

		{// Infrom physics world about changes by game engine
			auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
			for (auto&& [entityID, cc, rb, tc] : circles.each())
			{
				if (!mReg.all_of<NativeScriptComponent>(entityID))
					continue;
				glm::vec3 pos, scale, rotation;
				math::DecomposeTransform(tc, pos, scale, rotation);
				InformPhysicsWorld(rb, &cc, pos, rotation.z);
				b2CircleShape* shape = (b2CircleShape*)((b2Fixture*)cc.Fixure)->GetShape();
				shape->m_p.Set(cc.Offset.x, cc.Offset.y);
				shape->m_radius = scale.x * cc.Radius;
			}
			auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
			for (auto&& [entityID, bc, rb, tc] : boxes.each())
			{
				if (!mReg.all_of<NativeScriptComponent>(entityID))
					continue;
				glm::vec3 pos, scale, rotation;
				math::DecomposeTransform(tc, pos, scale, rotation);
				InformPhysicsWorld(rb, &bc, pos, rotation.z);
				b2PolygonShape* shape = (b2PolygonShape*)((b2Fixture*)bc.Fixure)->GetShape();
				shape->SetAsBox(scale.x * bc.Size.x, scale.y * bc.Size.y, { bc.Offset.x, bc.Offset.y }, 0.0f);
			}
		}
	}

	void Scene::Movement(float dt, bool physics)
	{
		glm::vec2 gravity = FindEntityWithUUID({}).GetComponent<Settings>().Gravity;
		auto group = mReg.group<Rigidbody2DComponent>(entt::get<Transform2DComponent, TransformationComponent>);
		for (auto&& [entityID, rb, tc, transform] : group.each())
		{
			if (rb.Type == BodyType::Static)
				continue;
			if (b2Body* body = (b2Body*)rb.Body) { if (rb.Type == BodyType::Dynamic && !body->IsAwake()) continue; }

			float deltaTime = dt;
			if (physics && rb.Body)
				deltaTime = mAccumulator;
			
			glm::vec2 g = gravity * rb.GravityFactor;
			if (rb.Type == BodyType::Dynamic && rb.Body == nullptr)//Hacking my way for moving in between box2D frames
				rb.Velocity += g * deltaTime;

			glm::vec3 pos, scale, rotation;
			math::DecomposeTransform(transform, pos, scale, rotation);
			pos += glm::vec3(rb.Velocity.x, rb.Velocity.y, 0.0f) * deltaTime;
			if(rb.Type == BodyType::Dynamic)
				pos += 0.5f * glm::vec3(g.x, g.y, 0.0f) * deltaTime * deltaTime;
			rotation.z += glm::radians(rb.AngularVelocity) * deltaTime;
			
			glm::mat4 pTransform;
			if (HasParentTransform(mReg.get<RelationshipComponent>(entityID), mReg, pTransform))
			{
				glm::mat4 world = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), rotation.z, { 0.0f, 0.0f, 1.0f });
				glm::mat4 local = glm::inverse(pTransform) * world;
				glm::vec3 lpos, lscale, lrotation;
				math::DecomposeTransform(local, lpos, lscale, lrotation);
				tc.Position = { lpos.x, lpos.y, tc.Position.z };
				tc.Rotation = glm::degrees(lrotation.z);
			}
			else
			{
				tc.Position = { pos.x, pos.y, tc.Position.z };
				tc.Rotation = glm::degrees(rotation.z);
			}
		}
	}

	void Scene::InformEngine(entt::entity entityID, Rigidbody2DComponent& rb, TransformationComponent& tc)
	{
		const auto& relc = mReg.get<RelationshipComponent>(entityID);
		glm::mat4 pTransform;
		bool World = HasParentTransform(relc, mReg, pTransform);
		b2Body* body = (b2Body*)rb.Body;
		if (World)
		{
			glm::mat4 world = glm::translate(glm::mat4(1.0f), { body->GetPosition().x, body->GetPosition().y, tc.Transform[3].z }) *
				glm::rotate(glm::mat4(1.0f), body->GetAngle(), { 0.0f, 0.0f, 1.0f });
			world = glm::inverse(pTransform) * world;
			glm::vec3 pos, scale, rotation;
			math::DecomposeTransform(world, pos, scale, rotation);
			auto& transform = mReg.get<Transform2DComponent>(entityID);
			transform.Position = { pos.x , pos.y, transform.Position.z };
			transform.Rotation = glm::degrees(rotation.z);
		}
		else
		{
			auto& transform = mReg.get<Transform2DComponent>(entityID);
			transform.Position = { body->GetPosition().x, body->GetPosition().y, transform.Position.z };
			transform.Rotation = glm::degrees(body->GetAngle());
		}
		rb.Velocity = { body->GetLinearVelocity().x , body->GetLinearVelocity().y };
		rb.AngularVelocity = glm::degrees(body->GetAngularVelocity());
	}

	void Scene::InformPhysicsWorld(Rigidbody2DComponent& rb, Collider* collider, const glm::vec3& pos, float angle)
	{
		b2Body* body = (b2Body*)rb.Body;
		body->SetTransform({ pos.x, pos.y }, angle);
		body->SetLinearVelocity({ rb.Velocity.x, rb.Velocity.y });
		body->SetAngularVelocity(glm::radians(rb.AngularVelocity));
		body->SetGravityScale(rb.GravityFactor);
		body->SetFixedRotation(rb.FixedRotation);
		body->SetBullet(rb.Bullet);

		b2Fixture* fixture = (b2Fixture*)collider->Fixure;
		fixture->SetDensity(collider->Density);
		fixture->SetFriction(collider->Friction);
		fixture->SetRestitution(collider->Restitution);
		fixture->SetRestitutionThreshold(collider->RestitutionThreshold);
	}

	void Scene::OnPhysicsStart(void)
	{
		Entity me = FindEntityWithUUID({});
		const glm::vec2& g = me.GetComponent<Settings>().Gravity;
		mPhysicsWorld = new b2World({ g.x, g.y });
		mPhysicsWorld->SetContactListener(CollisionDispatcher::Get());

		auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
		for (auto&& [entityID, cc, rb, tc] : circles.each())
		{
			glm::vec3 pos, scale, rotation;
			gte::math::DecomposeTransform(tc, pos, scale, rotation);
			b2BodyDef bodyDef = CreateBody(rb, pos, rotation.z);
			b2Body* body = mPhysicsWorld->CreateBody(&bodyDef);
			rb.Body = body;
			b2CircleShape shape;
			shape.m_p.Set(cc.Offset.x, cc.Offset.y);
			shape.m_radius = scale.x * cc.Radius;
			b2FixtureDef fixtureDef;
			fixtureDef.userData.pointer = static_cast<uintptr_t>(entityID);
			fixtureDef.shape = &shape;
			fixtureDef.density = cc.Density;
			fixtureDef.friction = cc.Friction;
			fixtureDef.restitution = cc.Restitution;
			fixtureDef.restitutionThreshold = cc.RestitutionThreshold;
			fixtureDef.isSensor = cc.Sensor;
			cc.Fixure = body->CreateFixture(&fixtureDef);
		}

		auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
		for (auto&& [entityID, bc, rb, tc] : boxes.each())
		{
			glm::vec3 pos, scale, rotation;
			gte::math::DecomposeTransform(tc, pos, scale, rotation);
			b2BodyDef bodyDef = CreateBody(rb, pos, rotation.z);
			b2Body* body = mPhysicsWorld->CreateBody(&bodyDef);
			rb.Body = body;
			b2PolygonShape shape;
			shape.SetAsBox(scale.x * bc.Size.x, scale.y * bc.Size.y, { bc.Offset.x, bc.Offset.y }, 0.0f);
			b2FixtureDef fixtureDef;
			fixtureDef.userData.pointer = static_cast<uintptr_t>(entityID);
			fixtureDef.shape = &shape;
			fixtureDef.density = bc.Density;
			fixtureDef.friction = bc.Friction;
			fixtureDef.restitution = bc.Restitution;
			fixtureDef.restitutionThreshold = bc.RestitutionThreshold;
			fixtureDef.isSensor = bc.Sensor;
			bc.Fixure = body->CreateFixture(&fixtureDef);
		}
	}

	void Scene::OnPhysicsStop(void)
	{
		delete mPhysicsWorld;
		mPhysicsWorld = nullptr;
	}

	void Scene::PatchScripts(void)
	{
		using namespace internal;
		auto scripts = mReg.view<NativeScriptComponent>();
		for (auto&& [entityID, nc] : scripts.each())
		{
			if (((NativeScript*)nc.ScriptAsset->Data)->GetVersion() > nc.Description.GetVersion())
			{
				const auto& oldSpecs = nc.Description.GetFieldsSpecification();
				void* oldBuffer = nc.Description.GetBuffer();
				auto newDescription = *(NativeScript*)nc.ScriptAsset->Data;
				const auto& specs = newDescription.GetFieldsSpecification();
				void* buffer = newDescription.GetBuffer();
				for (const auto& oldspec : oldSpecs)
				{
					void* srcPtr = (byte*)oldBuffer + oldspec.BufferOffset;
					for (const auto& spec : specs)
					{
						void* dstPtr = (byte*)buffer + spec.BufferOffset;
						if (spec.Name.compare(oldspec.Name) == 0 && oldspec.Type == spec.Type)
						{
							switch (spec.Type)
							{
							case FieldType::Bool:
							case FieldType::Enum_Char:
							case FieldType::Char:
							case FieldType::Enum_Byte:
							case FieldType::Byte:
							case FieldType::Enum_Int16:
							case FieldType::Int16:
							case FieldType::Enum_Int32:
							case FieldType::Int32:
							case FieldType::Enum_Int64:
							case FieldType::Int64:
							case FieldType::Enum_Uint16:
							case FieldType::Uint16:
							case FieldType::Enum_Uint32:
							case FieldType::Uint32:
							case FieldType::Enum_Uint64:
							case FieldType::Uint64:
							case FieldType::Float32:
							case FieldType::Float64:
							case FieldType::Vec2:
							case FieldType::Vec3:
							case FieldType::Vec4:
							case FieldType::Entity:
								//Can be trivially copied
								memcpy(dstPtr, srcPtr, spec.Size);
								break;
							case FieldType::String:
								*(std::string*)dstPtr = *(std::string*)srcPtr;
								break;
							case FieldType::Asset:
							{
								uuid assetID = (*((Ref<Asset>*)srcPtr))->ID;
								Ref<Asset>& ref = *((Ref<Asset>*)dstPtr);
								ref->ID = assetID;
								break;
							}
							case FieldType::Unknown:
								break;
							}
						}
					}
				}
				nc.Description = newDescription;
			}
		}
	}

}

void CreateTransform2D(entt::registry& reg, entt::entity entityID)
{
	const auto& tc = reg.get<gte::Transform2DComponent>(entityID);
	auto& transform = reg.emplace<gte::TransformationComponent>(entityID);

	glm::mat4 transformation = glm::translate(glm::mat4(1.0f), tc.Position) * glm::rotate(glm::mat4(1.0f), glm::radians(tc.Rotation), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { tc.Scale.x, tc.Scale.y, 1.0f });

	glm::mat4 pTransform;
	if (reg.all_of<gte::RelationshipComponent>(entityID) && HasParentTransform(reg.get<gte::RelationshipComponent>(entityID), reg, pTransform))
		transformation = pTransform * transformation;
	transform.Transform = transform;
}

void DestroyTransform2D(entt::registry& reg, entt::entity entityID)
{
	if (reg.all_of<gte::TransformationComponent>(entityID))
		reg.remove<gte::TransformationComponent>(entityID);
}

void CreateCamera(entt::registry& reg, entt::entity entityID) { reg.emplace<gte::OrthographicCameraComponent>(entityID); }

void DestroyCamera(entt::registry& reg, entt::entity entityID)
{
	if (reg.all_of<gte::OrthographicCameraComponent>(entityID))
		reg.remove<gte::OrthographicCameraComponent>(entityID);
}

b2BodyDef CreateBody(const gte::Rigidbody2DComponent& rb, const glm::vec3& pos, float angle)
{
	b2BodyDef bodyDef;
	bodyDef.position.Set(pos.x, pos.y);
	bodyDef.linearVelocity.Set(rb.Velocity.x, rb.Velocity.y);
	bodyDef.angle = angle;
	bodyDef.angularVelocity = glm::radians(rb.AngularVelocity);
	bodyDef.gravityScale = rb.GravityFactor;
	bodyDef.fixedRotation = rb.FixedRotation;
	bodyDef.bullet = rb.Bullet;
	switch (rb.Type)
	{
	case gte::BodyType::Static:
		bodyDef.type = b2_staticBody;
		break;
	case gte::BodyType::Kinematic:
		bodyDef.type = b2_kinematicBody;
		break;
	case gte::BodyType::Dynamic:
		bodyDef.type = b2_dynamicBody;
		break;
	}
	return bodyDef;
}

bool HasParentTransform(const gte::RelationshipComponent& relc, const entt::registry& reg, glm::mat4& pTransform)
{
	bool hasParent = false;
	if (relc.Parent != entt::null)
	{
		if (reg.all_of<gte::TransformationComponent>(relc.Parent))
		{
			pTransform = reg.get<gte::TransformationComponent>(relc.Parent);
			hasParent = true;
		}
		else
		{
			auto parent = reg.get<gte::RelationshipComponent>(relc.Parent).Parent;
			while (parent != entt::null && reg.all_of<gte::TransformationComponent>(parent))
				parent = reg.get<gte::RelationshipComponent>(parent).Parent;
			if (parent != entt::null && reg.all_of<gte::TransformationComponent>(parent))
			{
				pTransform = reg.get<gte::TransformationComponent>(parent);
				hasParent = true;
			}
		}
	}
	return hasParent;
}