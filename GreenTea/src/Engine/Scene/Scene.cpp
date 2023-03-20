#include "Scene.h"
#include "Components.h"
#include "CollisionDispatcher.h"
#include "Entity.h"

#include <Engine/Assets/Prefab.h>
#include <Engine/Core/Context.h>
#include <Engine/Core/Math.h>
#include <Engine/NativeScripting/ScriptableEntity.h>
#include <Engine/Renderer/Renderer2D.h>

//glm
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/compatibility.hpp>
// Box2D
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
//openal-soft
#include <AL/al.h>

static void CreateTransform2D(entt::registry& reg, entt::entity entityID);
static void DestroyTransform2D(entt::registry& reg, entt::entity entityID);

static void CreateCamera(entt::registry& reg, entt::entity entityID);
static void DestroyCamera(entt::registry& reg, entt::entity entityID);

static void CreateParticleSystem(entt::registry& reg, entt::entity entityID);
static void DestroyParticleSystem(entt::registry& reg, entt::entity entityID);

static void DestroyRigidbody2D(entt::registry& reg, entt::entity entityID);
static void DestroyCollider(entt::registry& reg, entt::entity entityID);

static b2BodyDef CreateBody(const gte::Rigidbody2DComponent& rb, const glm::vec3& pos, float angle);
static void SetListener(entt::registry& reg, entt::entity entity);

namespace gte {

	void Scene::Update(float dt)
	{
		internal::GetContext()->GlobalTime += dt;
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
		UpdateMatrices();

		//Handle Scripting logic
		std::vector<entt::entity> bin;
		{
#ifndef GT_DIST
			internal::ScopedSETranslator translator(internal::TranslateFunction);
#endif
			auto scripts = mReg.view<NativeScriptComponent>(entt::exclude<filters::Disabled>);
			for (auto&& [entityID, nc] : scripts.each())
			{
				if (nc.State == ScriptState::MustBeInitialized)
				{
					auto name = nc.Description.GetName();
					std::replace(name.begin(), name.end(), ' ', '_');
					try
					{
						nc.Instance = internal::GetContext()->DynamicLoader.CreateInstance<ScriptableEntity>(name);
						internal::GetContext()->ScriptEngine->Instantiate(nc.Instance, nc.Description);
						nc.Instance->mEntity = { entityID, this };
						nc.Instance->Start();
						nc.State = ScriptState::Active;
					}
					catch (RuntimeException e)
					{
						nc.State = ScriptState::Inactive;
						if (nc.Instance)
							delete nc.Instance;
						bin.emplace_back(entityID);
					}
					catch (AssertException e)
					{
						nc.State = ScriptState::Inactive;
						if (nc.Instance)
							delete nc.Instance;
						bin.emplace_back(entityID);
					}
				}
				else if (nc.State == ScriptState::Active)
				{
					try { nc.Instance->Update(dt); }
					catch (RuntimeException e) { nc.State = ScriptState::MustBeDestroyed; }
					catch (AssertException e) { nc.State = ScriptState::MustBeDestroyed; }
				}
			}

			auto destructables = mReg.view<filters::Destructable>();
			for (auto entityID : destructables)
			{
				const auto& rel = mReg.get<RelationshipComponent>(entityID);
				if (!destructables.contains(rel.Parent))
					bin.emplace_back(entityID);
				if (auto* nsc = mReg.try_get<NativeScriptComponent>(entityID))
				{
					try { nsc->Instance->Destroy(); }
					catch (RuntimeException e) {}
					catch (AssertException e) {}
					delete nsc->Instance;
				}
			}
		}

		for (auto entityID : bin)
			DestroyEntity({ entityID, this });

		{//Handle entities that were disabled or enabled
			auto bodies = mReg.view<Rigidbody2DComponent, filters::Disabled>();
			for (auto&& [entityID, rb] : bodies.each())
			{
				if (!rb.Body) continue;
				mPhysicsWorld->DestroyBody((b2Body*)rb.Body);
				rb.Body = nullptr;
			}

			SetupPhysics();
		}

		UpdateMatrices();
		InformAudioEngine();

		auto animations = mReg.view<AnimationComponent, SpriteRendererComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, ac, sprite] : animations.each())
		{
			ac.Animation = internal::GetContext()->AssetManager.RequestAsset(ac.Animation->ID);
			if (ac.Animation->Type != AssetType::ANIMATION)
				continue;

			internal::Animation& animation = *(internal::Animation*)ac.Animation->Data;
			if (animation != ac.Description)
				ac.Description = animation;
			else
				ac.Description.Update(dt);

			Ref<Asset> asset = internal::GetContext()->AssetManager.RequestAsset(ac.Description.GetAtlas());
			if (asset->Type == AssetType::TEXTURE)
			{
				const auto& frame = ac.Description.GetCurrentFrame();
				sprite.Coordinates = frame.Coords;
				sprite.Texture = asset;
			}
		}

		auto particles = mReg.view<ParticleSystemComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, psc] : particles.each())
			psc.System->SetProps(psc.Props);

		if (Entity camera = GetPrimaryCameraEntity())
			SetListener(mReg, camera);
	}

	void Scene::Render(const glm::mat4& eyematrix)
	{
		entt::insertion_sort algo;
		mReg.sort<TransformationComponent>([](const auto& lhs, const auto& rhs) { return lhs.Transform[3].z < rhs.Transform[3].z; }, algo);
		Renderer2D::BeginScene(eyematrix);
		auto view = mReg.view<TransformationComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, tc] : view.each())
		{
			if (auto* sprite = mReg.try_get<SpriteRendererComponent>(entityID))
			{
				if (!sprite->Visible)
					continue;
				sprite->Texture = internal::GetContext()->AssetManager.RequestAsset(sprite->Texture->ID);
				if (sprite->Texture->Type == AssetType::INVALID)
					Renderer2D::DrawQuad(tc.Transform, (uint32)entityID, sprite->Color);
				else if (sprite->Texture->Type == AssetType::TEXTURE)
				{
					TextureCoordinates coords = sprite->Coordinates;
					if (sprite->FlipX)
					{
						float x = coords.BottomLeft.x;
						coords.BottomLeft.x = coords.TopRight.x;
						coords.TopRight.x = x;
					}
					if (sprite->FlipY)
					{
						float y = coords.BottomLeft.y;
						coords.BottomLeft.y = coords.TopRight.y;
						coords.TopRight.y = y;
					}
					Renderer2D::DrawQuad(tc.Transform, (GPU::Texture*)sprite->Texture->Data, coords, (uint32)entityID, sprite->Color, sprite->TilingFactor);
				}
			}
			else if (auto* circle = mReg.try_get<CircleRendererComponent>(entityID))
			{
				if(circle->Visible)
					Renderer2D::DrawCircle(tc.Transform, circle->Color, (uint32)entityID, circle->Thickness, circle->Fade);
			}
				
		}

		if (internal::GetContext()->Playing)
		{
			auto particles = mReg.view<ParticleSystemComponent, TransformationComponent>(entt::exclude<filters::Disabled>);
			for (auto&& [entityID, psc, tc] : particles.each())
				psc.System->Render(tc);
		}

		auto texts = mReg.view<TransformationComponent, TextRendererComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, tc, text] : texts.each())
		{
			text.Font = internal::GetContext()->AssetManager.RequestAsset(text.Font->ID);
			if (text.Font->Type != AssetType::TEXTURE || !text.Visible)
				continue;
			Ref<Asset> font = internal::GetContext()->AssetManager.RequestAsset(text.Font->ID, true);
			Renderer2D::DrawString(text.Text, tc, text.Size, (GPU::Texture*)text.Font->Data, (internal::Font*)font->Data, text.Color);
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
		mReg.emplace<Transform2DComponent>(entityID);
		return { entityID, this };
	}

	Entity Scene::CreateChildEntity(Entity parent)
	{
		Entity entity = CreateEntity({});
		auto& prel = parent.GetComponent<RelationshipComponent>();
		auto nextID = prel.FirstChild;
		prel.FirstChild = entity;
		prel.Childrens++;

		auto& relc = entity.GetComponent<RelationshipComponent>();
		relc.Parent = parent;
		relc.Next = nextID;
		if (nextID != entt::null)
			mReg.get<RelationshipComponent>(nextID).Previous = entity;

		UpdateTransform(entity);
		return entity;
	}

	Entity Scene::CreateEntityFromPrefab(Ref<Asset> prefab, Entity parent)
	{
		Prefab* fab = (Prefab*)prefab->Data;
		const YAML::Node& entities = fab->GetNode();

		std::unordered_map<uuid, Entity> map;
		for (const auto& entityNode : entities)
		{
			uuid id = entityNode["Entity"].as<std::string>();
			std::string name = entityNode["TagComponent"]["Tag"].as<std::string>();
			Entity entity = CreateEntity(name);
			map.insert({ id, entity });

			const auto& transform = entityNode["Transform2DComponent"];
			if (transform)
			{
				auto& tc = entity.GetComponent<Transform2DComponent>();
				tc.Position = transform["Position"].as<glm::vec3>();
				tc.Scale = transform["Scale"].as<glm::vec2>();
				tc.Rotation = transform["Rotation"].as<float>();
			}

			const auto& renderable = entityNode["SpriteRendererComponent"];
			if (renderable)
			{
				auto& sprite = entity.AddComponent<SpriteRendererComponent>();
				sprite.Color = renderable["Color"].as<glm::vec4>();
				sprite.Visible = renderable["Visible"].as<bool>();
				uuid texID = renderable["Texture"].as<std::string>();
				if (texID.IsValid())
				{
					sprite.Texture->ID = texID;
					sprite.TilingFactor = renderable["TilingFactor"].as<float>();
					sprite.FlipX = renderable["FlipX"].as<bool>();
					sprite.FlipY = renderable["FlipY"].as<bool>();
					const auto& coords = renderable["TextureCoordinates"];
					sprite.Coordinates.BottomLeft = coords["BottomLeft"].as<glm::vec2>();
					sprite.Coordinates.TopRight = coords["TopRight"].as<glm::vec2>();
				}
			}

			const auto& circleRenderable = entityNode["CircleRendererComponent"];
			if (circleRenderable)
			{
				auto& circle = entity.AddComponent<CircleRendererComponent>();
				circle.Color = circleRenderable["Color"].as<glm::vec4>();
				circle.Thickness = circleRenderable["Thickness"].as<float>();
				circle.Fade = circleRenderable["Fade"].as<float>();
				circle.Visible = circleRenderable["Visible"].as<bool>();
			}

			const auto& textRenderer = entityNode["TextRendererComponent"];
			if (textRenderer)
			{
				auto& tc = entity.AddComponent<TextRendererComponent>();
				tc.Text = textRenderer["Text"].as<std::string>();
				tc.Color = textRenderer["Color"].as<glm::vec4>();
				tc.Font->ID = textRenderer["Font"].as<std::string>();
				tc.Size = textRenderer["Size"].as<uint32>();
				tc.Visible = textRenderer["Visible"].as<bool>();
			}

			const auto& camera = entityNode["CameraComponent"];
			if (camera)
			{
				auto& cam = entity.AddComponent<CameraComponent>();
				cam.Primary = camera["Primary"].as<bool>();
				cam.FixedAspectRatio = camera["FixedAspectRatio"].as<bool>();
				if (cam.FixedAspectRatio)
					cam.AspectRatio = camera["AspectRatio"].as<float>();
				cam.MasterVolume = camera["MasterVolume"].as<float>();
				cam.Model = (DistanceModel)camera["DistanceModel"].as<uint16>();

				auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
				ortho.ZoomLevel = camera["ZoomLevel"].as<float>();
				ortho.VerticalBoundary = camera["VerticalBoundary"].as<float>();
			}

			const auto& rigidbody = entityNode["Rigidbody2DComponent"];
			if (rigidbody)
			{
				auto& rb = entity.AddComponent<Rigidbody2DComponent>();
				rb.Type = (BodyType)rigidbody["Type"].as<uint64>();
				rb.Velocity = rigidbody["Velocity"].as<glm::vec2>();
				rb.AngularVelocity = rigidbody["AngularVelocity"].as<float>();
				rb.GravityFactor = rigidbody["GravityFactor"].as<float>();
				rb.FixedRotation = rigidbody["FixedRotation"].as<bool>();
				rb.Bullet = rigidbody["Bullet"].as<bool>();
			}

			const auto& boxcollider = entityNode["BoxColliderComponent"];
			if (boxcollider)
			{
				auto& bc = entity.AddComponent<BoxColliderComponent>();
				bc.Offset = boxcollider["Offset"].as<glm::vec2>();
				bc.Size = boxcollider["Size"].as<glm::vec2>();
				bc.Density = boxcollider["Density"].as<float>();
				bc.Friction = boxcollider["Friction"].as<float>();
				bc.Restitution = boxcollider["Restitution"].as<float>();
				bc.RestitutionThreshold = boxcollider["RestitutionThreshold"].as<float>();
				bc.Sensor = boxcollider["Sensor"].as<bool>();
			}

			const auto& circlecollider = entityNode["CircleColliderComponent"];
			if (circlecollider)
			{
				auto& cc = entity.AddComponent<CircleColliderComponent>();
				cc.Radius = circlecollider["Radius"].as<float>();
				cc.Density = circlecollider["Density"].as<float>();
				cc.Friction = circlecollider["Friction"].as<float>();
				cc.Restitution = circlecollider["Restitution"].as<float>();
				cc.RestitutionThreshold = circlecollider["RestitutionThreshold"].as<float>();
				cc.Sensor = circlecollider["Sensor"].as<bool>();
			}

			const auto& speaker = entityNode["SpeakerComponent"];
			if (speaker)
			{
				auto& sc = entity.AddComponent<SpeakerComponent>();
				sc.AudioClip->ID = speaker["AudioClip"].as<std::string>();
				sc.Volume = speaker["Volume"].as<float>();
				sc.Pitch = speaker["Pitch"].as<float>();
				sc.RollOffFactor = speaker["RollOffFactor"].as<float>();
				sc.RefDistance = speaker["RefDistance"].as<float>();
				sc.MaxDistance = speaker["MaxDistance"].as<float>();
				sc.Looping = speaker["Looping"].as<bool>();
				if (const auto& play = speaker["PlayOnStart"])
					sc.PlayOnStart = play.as<bool>();

				if (sc.PlayOnStart && internal::GetContext()->Playing)
					sc.Source.Play();
			}

			const auto& particleSystem = entityNode["ParticleSystemComponent"];
			if (particleSystem)
			{
				auto& psc = entity.AddComponent<ParticleSystemComponent>();
				psc.Props.Position = particleSystem["Position"].as<glm::vec2>();
				psc.Props.Velocity = particleSystem["Velocity"].as<glm::vec2>();
				psc.Props.VelocityVariation = particleSystem["VelocityVariation"].as<glm::vec2>();
				psc.Props.ColorBegin = particleSystem["ColorBegin"].as<glm::vec4>();
				psc.Props.ColorEnd = particleSystem["ColorEnd"].as<glm::vec4>();
				psc.Props.SizeBegin = particleSystem["SizeBegin"].as<glm::vec2>();
				psc.Props.SizeEnd = particleSystem["SizeEnd"].as<glm::vec2>();
				psc.Props.Rotation = particleSystem["Rotation"].as<float>();
				psc.Props.AngularVelocity = particleSystem["AngularVelocity"].as<float>();
				psc.Props.AngularVelocityVariation = particleSystem["AngularVelocityVariation"].as<float>();
				psc.Props.Duration = particleSystem["Duration"].as<float>();
				psc.Props.LifeTime = particleSystem["LifeTime"].as<float>();
				psc.Props.EmitionRate = particleSystem["EmitionRate"].as<float>();
				psc.Props.MaxParticles = particleSystem["MaxParticles"].as<uint32>();
				psc.Props.Looping = particleSystem["Looping"].as<bool>();
				if (const auto& play = particleSystem["PlayOnStart"])
					psc.PlayOnStart = play.as<bool>();
			}

			const auto& animation = entityNode["AnimationComponent"];
			if (animation)
			{
				auto& ac = entity.AddComponent<AnimationComponent>();
				ac.Animation->ID = animation["Animation"].as<std::string>();
			}
		}

		//Second iteration to create relationships & Native Scripts
		Entity toReturn;
		for (const auto& entityNode : entities)
		{
			uuid id = entityNode["Entity"].as<std::string>();
			Entity entity = map[id];

			const auto& relationship = entityNode["RelationshipComponent"];
			if (!relationship)//Special entity for Scene stuff
				continue;

			auto& rc = entity.GetComponent<RelationshipComponent>();
			rc.Childrens = relationship["Childrens"].as<uint64>();
			uuid candidate = relationship["FirstChild"].as<std::string>();
			if (candidate.IsValid())
				rc.FirstChild = (entt::entity)map[candidate];
				
			candidate = relationship["Previous"].as<std::string>();
			if (candidate.IsValid())
				rc.Previous = (entt::entity)map[candidate];

			candidate = relationship["Next"].as<std::string>();
			if (candidate.IsValid())
				rc.Next = (entt::entity)map[candidate];

			candidate = relationship["Parent"].as<std::string>();
			if (candidate.IsValid())
				rc.Parent = (entt::entity)map[candidate];
			else
				toReturn = entity;

			const auto& nativescript = entityNode["NativeScriptComponent"];
			if (nativescript)
			{
				const auto& props = nativescript["Properties"];
				auto& nc = entity.AddComponent<NativeScriptComponent>();
				uuid id = nativescript["Asset"].as<std::string>();
				nc.ScriptAsset = internal::GetContext()->AssetManager.RequestAsset(id);
				uint64 oldversion = nativescript["Version"].as<uint64>();
				if (((internal::NativeScript*)nc.ScriptAsset->Data)->GetVersion() > oldversion)
				{
					nc.Description = *(internal::NativeScript*)nc.ScriptAsset->Data;
					const auto& specs = nc.Description.GetFieldsSpecification();
					void* buffer = nc.Description.GetBuffer();
					for (const auto& prop : props)
					{
						const auto name = prop["Name"].as<std::string>();
						internal::FieldType type = (internal::FieldType)prop["Type"].as<uint64>();
						for (const auto& spec : specs)
						{
							void* ptr = (byte*)buffer + spec.BufferOffset;
							if (spec.Name.compare(name) == 0 && type == spec.Type)
							{
								switch (type)
								{
								using namespace internal;
								case FieldType::Bool:
									*(bool*)ptr = prop["Default"].as<bool>();
									break;
								case FieldType::Char:
								case FieldType::Enum_Char:
									*(char*)ptr = (char)prop["Default"].as<int16>();
									break;
								case FieldType::Enum_Byte:
								case FieldType::Byte:
									*(byte*)ptr = (byte)prop["Default"].as<byte>();
									break;
								case FieldType::Enum_Int16:
								case FieldType::Int16:
									*(int16*)ptr = prop["Default"].as<int16>();
									break;
								case FieldType::Enum_Int32:
								case FieldType::Int32:
									*(int32*)ptr = prop["Default"].as<int32>();
									break;
								case FieldType::Enum_Int64:
								case FieldType::Int64:
									*(int64*)ptr = prop["Default"].as<int64>();
									break;
								case FieldType::Enum_Uint16:
								case FieldType::Uint16:
									*(uint16*)ptr = prop["Default"].as<uint16>();
									break;
								case FieldType::Enum_Uint32:
								case FieldType::Uint32:
									*(uint32*)ptr = prop["Default"].as<uint32>();
									break;
								case FieldType::Enum_Uint64:
								case FieldType::Uint64:
									*(uint64*)ptr = prop["Default"].as<uint64>();
									break;
								case FieldType::Float32:
									*(float*)ptr = prop["Default"].as<float>();
									break;
								case FieldType::Float64:
									*(double*)ptr = prop["Default"].as<double>();
									break;
								case FieldType::Vec2:
									*(glm::vec2*)ptr = prop["Default"].as<glm::vec2>();
									break;
								case FieldType::Vec3:
									*(glm::vec3*)ptr = prop["Default"].as<glm::vec3>();
									break;
								case FieldType::Vec4:
									*(glm::vec4*)ptr = prop["Default"].as<glm::vec4>();
									break;
								case FieldType::String:
									*(std::string*)ptr = prop["Default"].as<std::string>();
									break;
								case FieldType::Asset:
								{
									uuid assetID = prop["Default"].as<std::string>();
									Ref<Asset>& ref = *((Ref<Asset>*)ptr);
									ref->ID = assetID;
									break;
								}
								case FieldType::Entity:
								{
									uuid entityID = prop["Default"].as<std::string>();
									if (map.find(entityID) == map.end())
										*(Entity*)ptr = FindEntityWithUUID(entityID);
									else
										*(Entity*)ptr = map[entityID];
									break;
								}
								case FieldType::Unknown:
									break;
								}
							}
						}
					}
				}
				else
				{
					nc.Description = *(internal::NativeScript*)nc.ScriptAsset->Data;
					const auto& specs = nc.Description.GetFieldsSpecification();
					void* buffer = nc.Description.GetBuffer();
					for (size_t i = 0; i < specs.size(); i++)
					{
						const auto& spec = specs[i];
						const auto& prop = props[i];
						void* ptr = (byte*)buffer + spec.BufferOffset;
						switch (spec.Type)
						{
						using namespace internal;
						case FieldType::Bool:
							*(bool*)ptr = prop["Default"].as<bool>();
							break;
						case FieldType::Char:
						case FieldType::Enum_Char:
							*(char*)ptr = (char)prop["Default"].as<int16>();
							break;
						case FieldType::Int16:
						case FieldType::Enum_Int16:
							*(int16*)ptr = prop["Default"].as<int16>();
							break;
						case FieldType::Int32:
						case FieldType::Enum_Int32:
							*(int32*)ptr = prop["Default"].as<int32>();
							break;
						case FieldType::Int64:
						case FieldType::Enum_Int64:
							*(int64*)ptr = prop["Default"].as<int64>();
							break;
						case FieldType::Byte:
						case FieldType::Enum_Byte:
							*(byte*)ptr = (byte)prop["Default"].as<uint16>();
							break;
						case FieldType::Uint16:
						case FieldType::Enum_Uint16:
							*(uint16*)ptr = prop["Default"].as<uint16>();
							break;
						case FieldType::Uint32:
						case FieldType::Enum_Uint32:
							*(uint32*)ptr = prop["Default"].as<uint32>();
							break;
						case FieldType::Uint64:
						case FieldType::Enum_Uint64:
							*(uint64*)ptr = prop["Default"].as<uint64>();
							break;
						case FieldType::Float32:
							*(float*)ptr = prop["Default"].as<float>();
							break;
						case FieldType::Float64:
							*(double*)ptr = prop["Default"].as<double>();
							break;
						case FieldType::Vec2:
							*(glm::vec2*)ptr = prop["Default"].as<glm::vec2>();
							break;
						case FieldType::Vec3:
							*(glm::vec3*)ptr = prop["Default"].as<glm::vec3>();
							break;
						case FieldType::Vec4:
							*(glm::vec4*)ptr = prop["Default"].as<glm::vec4>();
							break;
						case FieldType::String:
							*(std::string*)ptr = prop["Default"].as<std::string>();
							break;
						case FieldType::Asset:
						{
							uuid assetID = prop["Default"].as<std::string>();
							Ref<Asset>& ref = *((Ref<Asset>*)ptr);
							ref->ID = assetID;
							break;
						}
						case FieldType::Entity:
						{
							uuid entityID = prop["Default"].as<std::string>();
							if (map.find(entityID) == map.end())
								*(Entity*)ptr = FindEntityWithUUID(entityID);
							else
								*(Entity*)ptr = map[entityID];
							break;
						}
						case FieldType::Unknown:
							break;
						}
					}
				}
			}
		}

		if (parent)
		{
			auto& relc = toReturn.GetComponent<RelationshipComponent>();
			relc.Parent = parent;

			auto& prel = parent.GetComponent<RelationshipComponent>();
			prel.Childrens++;
			relc.Next = prel.FirstChild;
			prel.FirstChild = toReturn;
			if (relc.Next != entt::null)
				mReg.get<RelationshipComponent>(relc.Next).Previous = toReturn;
		}


		if (internal::GetContext()->Playing)
		{
			std::vector<Entity> bin;
#ifndef GT_DIST
			internal::ScopedSETranslator translator(internal::TranslateFunction);
#endif
			auto scripts = mReg.view<NativeScriptComponent>();
			for (auto [id, entity] : map)
			{
				if (!scripts.contains(entity))
					continue;

				auto [nsc] = scripts.get(entity);
				if (nsc.State == ScriptState::MustBeInitialized)
				{
					auto name = nsc.Description.GetName();
					std::replace(name.begin(), name.end(), ' ', '_');
					try
					{
						nsc.Instance = internal::GetContext()->DynamicLoader.CreateInstance<ScriptableEntity>(name);
						internal::GetContext()->ScriptEngine->Instantiate(nsc.Instance, nsc.Description);
						nsc.Instance->mEntity = entity;
						nsc.Instance->Start();
						nsc.State = ScriptState::Active;
					}
					catch (RuntimeException e)
					{
						nsc.State = ScriptState::Inactive;
						if (nsc.Instance)
							delete nsc.Instance;
						bin.emplace_back(entity);
					}
					catch (AssertException e)
					{
						nsc.State = ScriptState::Inactive;
						if (nsc.Instance)
							delete nsc.Instance;
						bin.emplace_back(entity);
					}
				}
			}
			for (auto entity : bin)
				DestroyEntity(entity);
		}

		if (!toReturn)//Check if entity still valid
			return {};

		UpdateTransform(toReturn);

		if (mPhysicsWorld)
		{
			auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
			for (auto [id, entity] : map)
			{
				if (!circles.contains(entity))
					continue;

				auto [cc, rb, tc] = circles.get(entity);
				glm::vec3 pos, scale, rotation;
				gte::math::DecomposeTransform(tc, pos, scale, rotation);
				{//Save previous Physics Tick on a different Registry
					auto entt = mPhysicsReg.create(entity);
					auto& prevTC = mPhysicsReg.emplace<Transform2DComponent>(entt);
					prevTC.Position = pos;
					prevTC.Rotation = rotation.z;
				}
				b2BodyDef bodyDef = CreateBody(rb, pos, rotation.z);
				b2Body* body = mPhysicsWorld->CreateBody(&bodyDef);
				rb.Body = body;
				b2CircleShape shape;
				shape.m_p.Set(cc.Offset.x, cc.Offset.y);
				shape.m_radius = scale.x * cc.Radius;
				b2FixtureDef fixtureDef;
				fixtureDef.userData.pointer = static_cast<uintptr_t>((entt::entity)entity);
				fixtureDef.shape = &shape;
				fixtureDef.density = cc.Density;
				fixtureDef.friction = cc.Friction;
				fixtureDef.restitution = cc.Restitution;
				fixtureDef.restitutionThreshold = cc.RestitutionThreshold;
				fixtureDef.isSensor = cc.Sensor;
				cc.Fixure = body->CreateFixture(&fixtureDef);
			}

			auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>);
			for (auto [id, entity] : map)
			{
				if (!boxes.contains(entity))
					continue;

				auto [bc, rb, tc] = boxes.get(entity);
				glm::vec3 pos, scale, rotation;
				gte::math::DecomposeTransform(tc, pos, scale, rotation);
				{//Save previous Physics Tick on a different Registry
					auto entt = mPhysicsReg.create(entity);
					auto& prevTC = mPhysicsReg.emplace<Transform2DComponent>(entt);
					prevTC.Position = pos;
					prevTC.Rotation = rotation.z;
				}
				b2BodyDef bodyDef = CreateBody(rb, pos, rotation.z);
				b2Body* body = mPhysicsWorld->CreateBody(&bodyDef);
				rb.Body = body;
				b2PolygonShape shape;
				shape.SetAsBox(scale.x * bc.Size.x, scale.y * bc.Size.y, { bc.Offset.x, bc.Offset.y }, 0.0f);
				b2FixtureDef fixtureDef;
				fixtureDef.userData.pointer = static_cast<uintptr_t>((entt::entity)entity);
				fixtureDef.shape = &shape;
				fixtureDef.density = bc.Density;
				fixtureDef.friction = bc.Friction;
				fixtureDef.restitution = bc.Restitution;
				fixtureDef.restitutionThreshold = bc.RestitutionThreshold;
				fixtureDef.isSensor = bc.Sensor;
				bc.Fixure = body->CreateFixture(&fixtureDef);
			}
		}
		return toReturn;
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
		Entity entity = CreateEntity({});
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

		uuid id = mReg.get<IDComponent>(entity);
		mReg.destroy(entity);
		if (mPhysicsReg.valid(entity))
			mPhysicsReg.destroy(entity);
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

	[[nodiscard]] std::vector<Entity> Scene::GetEntitiesByTag(const std::string& tag)
	{
		std::vector<Entity> entities;
		auto view = mReg.view<TagComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, tc] : view.each())
		{
			if (tc.Tag.compare(tag) == 0)
				entities.emplace_back(entityID, this);
		}
		return entities;
	}

	[[nodiscard]] Entity Scene::GetPrimaryCameraEntity(void)
	{
		Entity entity = {};

		auto cameras = mReg.view<CameraComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, cam] : cameras.each())
		{
			if (cam.Primary)
			{
				entity = { entityID, this };
				break;
			}
		}

		return entity;
	}

	void Scene::UpdateTransform(Entity entity)
	{
		const auto& tc = entity.GetComponent<Transform2DComponent>();
		auto& transform = entity.GetComponent<TransformationComponent>();
		transform.Transform = glm::translate(glm::mat4(1.0f), tc.Position) * glm::rotate(glm::mat4(1.0f), glm::radians(tc.Rotation), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { tc.Scale.x, tc.Scale.y, 1.0f });

		if (entity.HasComponent<RelationshipComponent>())
		{
			const auto& rel = entity.GetComponent<RelationshipComponent>();
			if (rel.Parent != entt::null)
			{
				const glm::mat4& pTransform = mReg.get<TransformationComponent>(rel.Parent);
				transform.Transform = pTransform * transform.Transform;
			}

			Entity child = { rel.FirstChild, this };
			for (size_t i = 0; i < rel.Childrens; i++)
			{
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

		mReg.on_construct<ParticleSystemComponent>().connect<CreateParticleSystem>();
		mReg.on_destroy<ParticleSystemComponent>().connect<DestroyParticleSystem>();

		mReg.on_destroy<Rigidbody2DComponent>().connect<&DestroyRigidbody2D>();
		mReg.on_destroy<BoxColliderComponent>().connect<&DestroyCollider>();
		mReg.on_destroy<CircleColliderComponent>().connect<&DestroyCollider>();

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
		auto view = mReg.view<RelationshipComponent>();
		for (auto&& [entityID, rel] : view.each())
		{
			if (rel.Parent != entt::null)
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
			const auto& srcTC = source.GetComponent<Transform2DComponent>();
			auto& dstTC = destination.GetComponent<Transform2DComponent>();
			dstTC = srcTC;

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

		if (source.HasComponent<SpeakerComponent>())
		{
			const auto& speaker = source.GetComponent<SpeakerComponent>();
			destination.AddComponent<SpeakerComponent>(speaker);
		}

		if (source.HasComponent<ParticleSystemComponent>())
		{
			const auto& psc = source.GetComponent<ParticleSystemComponent>();
			destination.AddComponent<ParticleSystemComponent>(psc);
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
				entt::entity entity = dstReg.create(entityID);//newScene->CreateEntityWithUUID(id, tag.Tag, false);
				dstReg.emplace<IDComponent>(entity, id);
				dstReg.emplace<TagComponent>(entity, tag);
				enttMap.insert({ id, entity });
				//entt ids' won't be valid since they are pointing source registry but they will be patch afterwards
				dstReg.emplace_or_replace<RelationshipComponent>(entity, srcReg.get<RelationshipComponent>(entityID));

				if (srcReg.any_of<filters::Disabled>(entityID))
					dstReg.emplace<filters::Disabled>(entity);
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
		{//Patch exported entities on NativeScripts
			auto view = dstReg.view<NativeScriptComponent>();
			for (auto&& [entityID, nsc] : view.each())
			{
				if (!nsc.ScriptAsset->ID.IsValid())
					continue;
				auto& specs = nsc.Description.GetFieldsSpecification();
				for (const auto& spec : specs)
				{
					if (spec.Type != internal::FieldType::Entity)
						continue;
					Entity* entity = (Entity*)((byte*)nsc.Description.GetBuffer() + spec.BufferOffset);
					auto entityID = (entt::entity)*entity;
					new (entity) Entity(entityID, newScene);
				}
			}
		}
		newScene->UpdateMatrices();
		return newScene;
	}

	void Scene::OnStart(void)
	{
		InformAudioEngine();
		std::vector<entt::entity> bin;
		{
#ifndef GT_DIST
			internal::ScopedSETranslator translator(internal::TranslateFunction);
#endif
			auto scripts = mReg.view<NativeScriptComponent>(entt::exclude<filters::Disabled>);
			for (auto&& [entityID, nc] : scripts.each())
			{
				if (nc.State == ScriptState::MustBeInitialized)
				{
					auto name = nc.Description.GetName();
					std::replace(name.begin(), name.end(), ' ', '_');
					try
					{
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
							bin.emplace_back(entityID);
						}
					}
					catch(RuntimeException e)
					{
						nc.State = ScriptState::Inactive;
						if (nc.Instance)
							delete nc.Instance;
						bin.emplace_back(entityID);
					}
					catch (AssertException e)
					{
						nc.State = ScriptState::Inactive;
						if (nc.Instance)
							delete nc.Instance;
						bin.emplace_back(entityID);
					}
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
		
		InformAudioEngine();
		auto speakers = mReg.view<SpeakerComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, speaker] : speakers.each())
		{
			if (speaker.PlayOnStart)
				speaker.Source.Play();
		}

		OnPhysicsStart();
		internal::GetContext()->GlobalTime = 0.0f;
	}

	void Scene::OnStop(void)
	{
		OnPhysicsStop();
		DestroyRuntime();
		internal::GetContext()->GlobalTime = 0.0f;
	}

	void Scene::DestroyRuntime()
	{
		auto scripts = mReg.view<NativeScriptComponent>();
		for (auto&& [entityID, nc] : scripts.each())
		{
			if (!nc.Instance)
				continue;
			try { nc.Instance->Destroy(); }
			catch (RuntimeException e) {}
			catch (AssertException e) {}
			delete nc.Instance;
			nc.Instance = nullptr;
		}
	}

	void Scene::FixedUpdate(void)
	{
		Entity me = FindEntityWithUUID({});
		auto& settings = me.GetComponent<Settings>();
		const float STEP = 1.0f / settings.Rate;

		{// Inform game engine about changes by physics world
			auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
			for (auto&& [entityID, cc, rb, tc] : circles.each())
			{
				if (rb.Type == BodyType::Static)
					continue;
				InformEngine(entityID, rb, tc);
			}
			auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
			for (auto&& [entityID, bc, rb, tc] : boxes.each())
			{
				if (rb.Type == BodyType::Static)
					continue;
				InformEngine(entityID, rb, tc);
			}
		}
		//Call FixedUpdate() on scripts
		auto scripts = mReg.view<NativeScriptComponent>(entt::exclude<filters::Disabled>);
		scripts.each([](auto& script) {
			if (script.State == ScriptState::Active)
				script.Instance->FixedUpdate();
		});

		{//Handle entities that were disabled or enabled
			auto bodies = mReg.view<Rigidbody2DComponent, filters::Disabled>();
			for (auto&& [entityID, rb] : bodies.each())
			{
				if (!rb.Body) continue;
				mPhysicsWorld->DestroyBody((b2Body*)rb.Body);
			}

			SetupPhysics();
		}

		UpdateMatrices();
		
		{// Infrom physics world about changes by game engine
			auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
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
			auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
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

		mPhysicsWorld->Step(STEP, settings.VelocityIterations, settings.PositionIterations);
	}

	void Scene::Movement(float dt, bool physics)
	{
		const auto& settings = FindEntityWithUUID({}).GetComponent<Settings>();
		const glm::vec2 gravity = settings.Gravity;
		const float tickRate = 1.0f / settings.Rate;
		const float deltaTime = mAccumulator / tickRate;

		auto group = mReg.group<Rigidbody2DComponent>(entt::get<Transform2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
		for (auto&& [entityID, rb, tc, transform] : group.each())
		{
			if (rb.Type == BodyType::Static)
				continue;

			//Update global Positions
			glm::vec3 pos, scale, rotation;
			math::DecomposeTransform(transform, pos, scale, rotation);
			if (b2Body* body = (b2Body*)rb.Body)
			{
				if (rb.Type == BodyType::Dynamic && !body->IsAwake())
					continue;
				
				const glm::vec3 targetPos = { body->GetPosition().x, body->GetPosition().y, pos.z };
				const float targetAngle = body->GetAngle();
				const auto& prevTC = mPhysicsReg.get<Transform2DComponent>(entityID);
				pos = glm::lerp(prevTC.Position, targetPos, deltaTime);
				rotation.z = glm::lerp(prevTC.Rotation, targetAngle, deltaTime);
			}
			else
			{
				const glm::vec2 g = gravity * rb.GravityFactor;
				if (rb.Type == BodyType::Dynamic)
					rb.Velocity += g * dt;
				pos += glm::vec3(rb.Velocity.x, rb.Velocity.y, 0.0f) * dt;
				if (rb.Type == BodyType::Dynamic)
					pos += 0.5f * glm::vec3(g.x, g.y, 0.0f) * dt * dt;
				rotation.z += glm::radians(rb.AngularVelocity) * dt;
			}
			
			//Update local positions
			if (auto* rc = mReg.try_get<RelationshipComponent>(entityID); rc->Parent != entt::null)
			{
				const glm::mat4& pTransform = mReg.get<TransformationComponent>(rc->Parent);
				const glm::mat4& world = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), rotation.z, { 0.0f, 0.0f, 1.0f });
				const glm::mat4& local = glm::inverse(pTransform) * world;
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

		auto view = mReg.view<ParticleSystemComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, psc] : view.each())
			psc.System->Update(dt);
	}

	void Scene::InformAudioEngine(void)
	{
		auto view = mReg.view<SpeakerComponent, TransformationComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, speaker, tc] : view.each())
		{
			speaker.AudioClip = internal::GetContext()->AssetManager.RequestAsset(speaker.AudioClip->ID);
			speaker.Source.SetProperties(&speaker);
			speaker.Source.SetPosition(glm::vec3(tc.Transform[3]));

			if (mReg.all_of<Rigidbody2DComponent>(entityID))
			{
				const auto& rb = mReg.get<Rigidbody2DComponent>(entityID);
				speaker.Source.SetVelocity(rb.Velocity);
			}
			else
				speaker.Source.SetVelocity({ 0.0f, 0.0f });
		}
	}

	void Scene::InformEngine(entt::entity entityID, Rigidbody2DComponent& rb, TransformationComponent& tc)
	{
		auto& prevTC = mPhysicsReg.get<Transform2DComponent>(entityID);
		const auto& relc = mReg.get<RelationshipComponent>(entityID);
		
		b2Body* body = (b2Body*)rb.Body;
		prevTC.Rotation = body->GetAngle();

		bool World = relc.Parent != entt::null;
		if (World)
		{
			const glm::mat4& pTransform = mReg.get<TransformationComponent>(relc.Parent);
			glm::mat4 world = glm::translate(glm::mat4(1.0f), { body->GetPosition().x, body->GetPosition().y, tc.Transform[3].z }) *
				glm::rotate(glm::mat4(1.0f), body->GetAngle(), { 0.0f, 0.0f, 1.0f });
			world = glm::inverse(pTransform) * world;
			glm::vec3 pos, scale, rotation;
			math::DecomposeTransform(world, pos, scale, rotation);
			auto& transform = mReg.get<Transform2DComponent>(entityID);
			transform.Position = { pos.x , pos.y, transform.Position.z };
			transform.Rotation = glm::degrees(rotation.z);
			prevTC.Position = { body->GetPosition().x, body->GetPosition().y, transform.Position.z };
		}
		else
		{
			auto& transform = mReg.get<Transform2DComponent>(entityID);
			transform.Position = { body->GetPosition().x, body->GetPosition().y, transform.Position.z };
			transform.Rotation = glm::degrees(body->GetAngle());
			prevTC.Position = { body->GetPosition().x, body->GetPosition().y, transform.Position.z };
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
		const auto& settings = me.GetComponent<Settings>();
		mPhysicsWorld = new b2World({ settings.Gravity.x, settings.Gravity.y });
		mPhysicsWorld->SetContactListener(CollisionDispatcher::Get());

		SetupPhysics();

		const float tickRate = 1.0f / settings.Rate;
		mPhysicsWorld->Step(tickRate, settings.VelocityIterations, settings.PositionIterations);
	}

	void Scene::OnPhysicsStop(void)
	{
		delete mPhysicsWorld;
		mPhysicsWorld = nullptr;
		mPhysicsReg.clear<Transform2DComponent>();
	}

	void Scene::SetupPhysics(void)
	{
		auto circles = mReg.group<CircleColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
		for (auto&& [entityID, cc, rb, tc] : circles.each())
		{
			glm::vec3 pos, scale, rotation;
			gte::math::DecomposeTransform(tc, pos, scale, rotation);
			if(!mPhysicsReg.valid(entityID))//Save previous Physics Tick on a different Registry
			{
				auto entt = mPhysicsReg.create(entityID);
				auto& prevTC = mPhysicsReg.emplace<Transform2DComponent>(entt);
				prevTC.Position = pos;
				prevTC.Rotation = rotation.z;
			}
			if (!rb.Body)
			{
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
		}

		auto boxes = mReg.group<BoxColliderComponent>(entt::get<Rigidbody2DComponent, TransformationComponent>, entt::exclude<filters::Disabled>);
		for (auto&& [entityID, bc, rb, tc] : boxes.each())
		{
			glm::vec3 pos, scale, rotation;
			gte::math::DecomposeTransform(tc, pos, scale, rotation);
			if (!mPhysicsReg.valid(entityID))//Save previous Physics Tick on a different Registry
			{
				auto entt = mPhysicsReg.create(entityID);
				auto& prevTC = mPhysicsReg.emplace<Transform2DComponent>(entt);
				prevTC.Position = pos;
				prevTC.Rotation = rotation.z;
			}
			if (!rb.Body)
			{
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
	}

	void Scene::PatchScripts(void)
	{
		using namespace internal;
		auto scripts = mReg.view<NativeScriptComponent>();
		for (auto&& [entityID, nc] : scripts.each())
		{
			if (!nc.ScriptAsset->ID.IsValid())
				continue;
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

	if (auto* rc = reg.try_get<gte::RelationshipComponent>(entityID); rc && rc->Parent != entt::null)
	{
		if(auto* pTransform = reg.try_get<gte::TransformationComponent>(rc->Parent))
			transformation = pTransform->Transform * transformation;
	}
	transform.Transform = transformation;
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

void CreateParticleSystem(entt::registry& reg, entt::entity entityID)
{
	if (!gte::internal::GetContext()->Playing)
		return;

	auto& psc = reg.get<gte::ParticleSystemComponent>(entityID);
	psc.System = new gte::internal::ParticleSystem(psc.Props);
	if(psc.PlayOnStart)
		psc.System->Start();
}

void DestroyParticleSystem(entt::registry& reg, entt::entity entityID)
{
	auto& psc = reg.get<gte::ParticleSystemComponent>(entityID);
	if (psc.System)
		delete psc.System;
}

void DestroyRigidbody2D(entt::registry& reg, entt::entity entityID)
{
	auto& rb = reg.get<gte::Rigidbody2DComponent>(entityID);
	if (rb.Body)
	{
		b2Body* body = (b2Body*)rb.Body;
		b2World* world = body->GetWorld();
		world->DestroyBody(body);
	}
}

void DestroyCollider(entt::registry& reg, entt::entity entityID)
{
	b2Fixture* fixture = nullptr;
	if (auto* cc = reg.try_get<gte::CircleColliderComponent>(entityID))
		fixture = (b2Fixture*)cc->Fixure;
	else if (auto* bc = reg.try_get<gte::BoxColliderComponent>(entityID))
		fixture = (b2Fixture*)bc->Fixure;
	if (auto* rb = reg.try_get<gte::Rigidbody2DComponent>(entityID))
	{
		if (rb->Body)
		{
			b2Body* body = (b2Body*)rb->Body;
			body->DestroyFixture(fixture);
		}
	}
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

void SetListener(entt::registry& reg, entt::entity entity)
{
	const auto& cam = reg.get<gte::CameraComponent>(entity);
	alListenerf(AL_GAIN, cam.MasterVolume);
	switch(cam.Model)
	{
	case gte::DistanceModel::None:
		alDistanceModel(AL_NONE);
		break;
	case gte::DistanceModel::Inverse:
		alDistanceModel(AL_INVERSE_DISTANCE);
		break;
	case gte::DistanceModel::InverseClamp:
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		break;
	case gte::DistanceModel::Linear:
		alDistanceModel(AL_LINEAR_DISTANCE);
		break;
	case gte::DistanceModel::LinearClamp:
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		break;
	case gte::DistanceModel::Exponent:
		alDistanceModel(AL_EXPONENT_DISTANCE);
		break;
	case gte::DistanceModel::ExponentClamp:
		alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
		break;
	}

	if (auto* tc = reg.try_get<gte::TransformationComponent>(entity))
		alListener3f(AL_POSITION, tc->Transform[3].x, tc->Transform[3].y, tc->Transform[3].z);
	else
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	if (auto* rb = reg.try_get<gte::Rigidbody2DComponent>(entity))
		alListener3f(AL_VELOCITY, rb->Velocity.x, rb->Velocity.y, 0.0f);
	else
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
}