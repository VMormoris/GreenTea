#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"

#include <Engine/Core/Context.h>
#include <Engine/Core/uuid.h>
#include <Engine/Core/Math.h>

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <time.h>

namespace gte::internal {

	SceneSerializer::SceneSerializer(Scene* scene)
		: mScene(scene) {}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		std::time_t result = std::time(nullptr);
		std::ofstream os(filepath, std::ios::binary);
		os << "# Scene Description for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 10 << '\n' << uuid::Create() << '\n';

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << std::filesystem::path(filepath).stem().string();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		mScene->mReg.each([&](auto entityID) {
			Entity entity = { entityID, mScene };
			if (!entity)
				return;
			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		os << out.size() << "\n\n";
		os << out.c_str();
		os.close();
	}

	void SceneSerializer::Deserialize(const std::string& filepath)
	{
		using namespace gte::math;
		uuid id;
		uint16_t loaded = 0;
		size_t size = 0;

		std::ifstream is(filepath);
		std::string line;
		while (getline(is, line))
		{
			if (line[0] == '#')//Comment ingore
				continue;
			else if (line.empty() || line[0] == '\n' || (line[0] == '\r' && line[1] == '\n'))
				break;
			if (loaded == 0) { ASSERT(stoul(line) == 10, "Asset type isn't Scene"); }
			else if (loaded == 1)
				id = line;
			else if (loaded == 2)
				size = stoull(line);
			loaded++;
		}

		char* buffer = new char[size + 1];
		buffer[size] = 0;
		is.read(buffer, size);
		is.close();

		YAML::Node data;
		try { data = YAML::Load(buffer); }
		catch (YAML::ParserException e) { ASSERT(false, "Failed to load file: %s\n\t%s\n", filepath.c_str(), e.what()); }
		delete[] buffer;

		//mScene->mReg = {};
		auto entities = data["Entities"];
		if (entities)
		{
			for (const auto& entityNode : entities)
			{
				//Get components that all entities should have (aka ID and Tag)
				uuid entityUUID = entityNode["Entity"].as<std::string>();
				std::string name;
				auto tagComponent = entityNode["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				
				if (!entityUUID.IsValid())//Special Entity for scene stuff
				{
					Entity entity = mScene->FindEntityWithUUID(entityUUID);
					const auto& transform = entityNode["TransformComponent"];
					auto& tc = entity.GetComponent<TransformComponent>();
					tc.Position = transform["Position"].as<glm::vec3>();
					tc.Scale = transform["Scale"].as<glm::vec3>();
					tc.Rotation = transform["Rotation"].as<glm::vec3>();

					const auto& camera = entityNode["CameraComponent"];
					auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
					ortho.ZoomLevel = camera["ZoomLevel"].as<float>();
					ortho.VerticalBoundary = camera["VerticalBoundary"].as<float>();
					auto& cam = entity.GetComponent<CameraComponent>();
					cam.Primary = camera["Primary"].as<bool>();
					cam.FixedAspectRatio = camera["FixedAspectRatio"].as<bool>();
					cam.MasterVolume = camera["MasterVolume"].as<float>();
					cam.Model = (DistanceModel)camera["DistanceModel"].as<uint16>();


					const auto& settings = entityNode["Settings"];
					auto& sett = entity.GetComponent<Settings>();
					sett.Gravity = settings["Gravity"].as<glm::vec2>();
					sett.Rate = settings["Rate"].as<int32>();
					sett.VelocityIterations = settings["VelocityIterations"].as<int32>();
					sett.PositionIterations = settings["PositionIterations"].as<int32>();
					sett.CameraVelocity = settings["CameraVelocity"].as<glm::vec2>();
					continue;
				}

				//Create entity
				Entity entity = mScene->CreateEntityWithUUID(entityUUID, name);

				//Check for other components expect Relationship
				const auto& transform = entityNode["Transform2DComponent"];
				if (transform)
				{
					auto& tc = entity.GetComponent<TransformComponent>();
					if (!entity.HasComponent<TransformationComponent>())//For some weird reason on_construct doesn't always work
						entity.AddComponent<TransformationComponent>();
					tc.Position = transform["Position"].as<glm::vec3>();
					tc.Scale = transform["Scale"].as<glm::vec3>();
					tc.Rotation = transform["Rotation"].as<glm::vec3>();
				}

				const auto& renderable = entityNode["SpriteRendererComponent"];
				if(renderable)
				{
					auto& sprite = entity.AddComponent<SpriteRendererComponent>();
					sprite.Color = renderable["Color"].as<glm::vec4>();
					sprite.Visible = renderable["Visible"].as<bool>();
					uuid textureID = renderable["Texture"].as<std::string>();
					if (textureID.IsValid())
					{
						sprite.Texture = internal::GetContext()->AssetManager.RequestAsset(textureID);
						sprite.TilingFactor = renderable["TilingFactor"].as<float>();
						sprite.FlipX = renderable["FilpX"].as<bool>();
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

				const auto& textRenderable = entityNode["TextRendererComponent"];
				if (textRenderable)
				{
					auto& tc = entity.AddComponent<TextRendererComponent>();
					tc.Text = textRenderable["Text"].as<std::string>();
					tc.Color = textRenderable["Color"].as<glm::vec4>();
					tc.Font->ID = textRenderable["Font"].as<std::string>();
					tc.Size = textRenderable["Size"].as<uint32>();
					tc.Visible = textRenderable["Visible"].as<bool>();
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
				if(circlecollider)
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
			for (const auto& entityNode : entities)
			{
				uuid entityUUID = entityNode["Entity"].as<std::string>();
				Entity entity = mScene->FindEntityWithUUID(entityUUID);

				const auto& relationship = entityNode["RelationshipComponent"];
				if (!relationship)//Special entity for Scene stuff
					continue;

				auto& rc = entity.GetComponent<RelationshipComponent>();
				rc.Childrens = relationship["Childrens"].as<uint64>();
				uuid candidate = relationship["FirstChild"].as<std::string>();
				if (candidate.IsValid())
					rc.FirstChild = (entt::entity)mScene->FindEntityWithUUID(candidate);

				candidate = relationship["Previous"].as<std::string>();
				if (candidate.IsValid())
					rc.Previous = (entt::entity)mScene->FindEntityWithUUID(candidate);

				candidate = relationship["Next"].as<std::string>();
				if (candidate.IsValid())
					rc.Next = (entt::entity)mScene->FindEntityWithUUID(candidate);

				candidate = relationship["Parent"].as<std::string>();
				if (candidate.IsValid())
					rc.Parent = (entt::entity)mScene->FindEntityWithUUID(candidate);

				const auto& nativescript = entityNode["NativeScriptComponent"];
				if (nativescript)
				{
					const auto& props = nativescript["Properties"];
					auto& nc = entity.AddComponent<NativeScriptComponent>();
					uuid scriptID = nativescript["Asset"].as<std::string>();
					nc.ScriptAsset = internal::GetContext()->AssetManager.RequestAsset(scriptID);
					uint64 oldversion = nativescript["Version"].as<uint64>();
					if (scriptID.IsValid() && ((NativeScript*)nc.ScriptAsset->Data)->GetVersion() > oldversion)
					{
						nc.Description = *(NativeScript*)nc.ScriptAsset->Data;
						const auto& specs = nc.Description.GetFieldsSpecification();
						void* buffer = nc.Description.GetBuffer();
						for (const auto& prop : props)
						{
							const auto name = prop["Name"].as<std::string>();
							FieldType type = (FieldType)prop["Type"].as<uint64>();
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
										*(Entity*)ptr = internal::GetContext()->ActiveScene->FindEntityWithUUID(entityID);
										break;
									}
									case FieldType::Unknown:
										break;
									}
								}
							}
						}
					}
					else if(scriptID.IsValid())
					{
						nc.Description = *(NativeScript*)nc.ScriptAsset->Data;
						const auto& specs = nc.Description.GetFieldsSpecification();
						void* buffer = nc.Description.GetBuffer();
						for (size_t i = 0; i < specs.size(); i++)
						{
							const auto& spec = specs[i];
							const auto& prop = props[i];
							void* ptr = (byte*)buffer + spec.BufferOffset;
							switch (spec.Type)
							{
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
								*(Entity*)ptr = internal::GetContext()->ActiveScene->FindEntityWithUUID(entityID);
								break;
							}
							case FieldType::Unknown:
								break;
							}
						}
					}
				}

			}

			mScene->UpdateMatrices();
		}
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, gte::Entity entity)
	{
		using namespace gte;
		using namespace gte::math;

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.GetID().str();
		if (entity.HasComponent<Settings>())
		{
			const auto& settings = entity.GetComponent<Settings>();
			out << YAML::Key << "Settings";
			out << YAML::BeginMap;
			out << YAML::Key << "Gravity" << YAML::Value << settings.Gravity;
			out << YAML::Key << "Rate" << YAML::Value << settings.Rate;
			out << YAML::Key << "VelocityIterations" << YAML::Value << settings.VelocityIterations;
			out << YAML::Key << "PositionIterations" << YAML::Value << settings.PositionIterations;
			out << YAML::Key << "CameraVelocity" << YAML::Value <<
				settings.CameraVelocity;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			const auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<RelationshipComponent>())
		{
			out << YAML::Key << "RelationshipComponent";
			out << YAML::BeginMap;

			const auto& rc = entity.GetComponent<RelationshipComponent>();
			out << YAML::Key << "Childrens" << YAML::Value << rc.Childrens;
			out << YAML::Key << "FirstChild" << YAML::Value << (rc.FirstChild == entt::null ? uuid().str() : Entity{ rc.FirstChild, mScene }.GetID().str());
			out << YAML::Key << "Previous" << YAML::Value << (rc.Previous == entt::null ? uuid().str() : Entity{ rc.Previous, mScene }.GetID().str());
			out << YAML::Key << "Next" << YAML::Value << (rc.Next == entt::null ? uuid().str() : Entity{ rc.Next, mScene }.GetID().str());
			out << YAML::Key << "Parent" << YAML::Value << (rc.Parent == entt::null ? uuid().str() : Entity{ rc.Parent, mScene }.GetID().str());
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "Transform2DComponent";
			out << YAML::BeginMap;

			const auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << tc.Position;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			const auto& sprite = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << sprite.Color;
			out << YAML::Key << "Visible" << YAML::Value << sprite.Visible;
			out << YAML::Key << "Texture" << YAML::Value << sprite.Texture->ID.str();
			if (sprite.Texture->ID.IsValid())
			{
				out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
				out << YAML::Key << "FilpX" << YAML::Value << sprite.FlipX;
				out << YAML::Key << "FlipY" << YAML::Value << sprite.FlipY;
				out << YAML::Key << "TextureCoordinates";
				out << YAML::BeginMap;
				out << YAML::Key << "BottomLeft" << YAML::Value << sprite.Coordinates.BottomLeft;
				out << YAML::Key << "TopRight" << YAML::Value << sprite.Coordinates.TopRight;
				out << YAML::EndMap;
			}
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			const auto& circle = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << circle.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circle.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circle.Fade;
			out << YAML::Key << "Visible" << YAML::Value << circle.Visible;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TextRendererComponent>())
		{
			const auto& tc = entity.GetComponent<TextRendererComponent>();
			out << YAML::Key << "TextRendererComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Text" << YAML::Value << tc.Text;
			out << YAML::Key << "Color" << YAML::Value << tc.Color;
			out << YAML::Key << "Font" << YAML::Value << tc.Font->ID.str();
			out << YAML::Key << "Size" << YAML::Value << tc.Size;
			out << YAML::Key << "Visible" << YAML::Value << tc.Visible;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			const auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
			const auto& cam = entity.GetComponent<CameraComponent>();
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "ZoomLevel" << YAML::Value << ortho.ZoomLevel;
			out << YAML::Key << "VerticalBoundary" << YAML::Value << ortho.VerticalBoundary;
			if(cam.FixedAspectRatio)
				out << YAML::Key << "AspectRatio" << YAML::Value << cam.AspectRatio;
			out << YAML::Key << "Primary" << YAML::Value << cam.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cam.FixedAspectRatio;
			out << YAML::Key << "MasterVolume" << YAML::Value << cam.MasterVolume;
			out << YAML::Key << "DistanceModel" << YAML::Value << (uint16)cam.Model;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap;

			const auto& rb = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "Type" << YAML::Value << (uint64)rb.Type;
			out << YAML::Key << "Velocity" << YAML::Value << rb.Velocity;
			out << YAML::Key << "AngularVelocity" << YAML::Value << rb.AngularVelocity;
			out << YAML::Key << "GravityFactor" << YAML::Value << rb.GravityFactor;
			out << YAML::Key << "FixedRotation" << YAML::Value << rb.FixedRotation;
			out << YAML::Key << "Bullet" << YAML::Value << rb.Bullet;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap;

			const auto& bc = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc.Size;
			out << YAML::Key << "Density" << YAML::Value << bc.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc.RestitutionThreshold;
			out << YAML::Key << "Sensor" << YAML::Value << bc.Sensor;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<CircleColliderComponent>())
		{
			out << YAML::Key << "CircleColliderComponent";
			out << YAML::BeginMap;

			const auto& cc = entity.GetComponent<CircleColliderComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc.RestitutionThreshold;
			out << YAML::Key << "Sensor" << YAML::Value << cc.Sensor;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap;

			const auto& nc = entity.GetComponent<NativeScriptComponent>();
			out << YAML::Key << "Asset" << YAML::Value << nc.ScriptAsset->ID.str();
			out << YAML::Key << "Version" << YAML::Value << nc.Description.GetVersion();
			out << YAML::Key << "Properties" << YAML::Value <<YAML::BeginSeq;
			const void* buffer = nc.Description.GetBuffer();
			for (const auto& prop : nc.Description.GetFieldsSpecification())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << YAML::Value << prop.Name;
				out << YAML::Key << "Type" << YAML::Value << (uint16)prop.Type;
				out << YAML::Key << "Default" << YAML::Value;
				const void* ptr = (const byte*)buffer + prop.BufferOffset;
				switch(prop.Type)
				{
				case FieldType::Bool:
					out << *(bool*)ptr;
					break;
				case FieldType::Char:
				case FieldType::Enum_Char:
					out << *(int16*)ptr;
					break;
				case FieldType::Int16:
				case FieldType::Enum_Int16:
					out << *(int16*)ptr;
					break;
				case FieldType::Int32:
				case FieldType::Enum_Int32:
					out << *(int32*)ptr;
					break;
				case FieldType::Int64:
				case FieldType::Enum_Int64:
					out << *(int64*)ptr;
					break;
				case FieldType::Byte:
				case FieldType::Enum_Byte:
					out << (uint16)*(byte*)ptr;
					break;
				case FieldType::Uint16:
				case FieldType::Enum_Uint16:
					out << *(uint16*)ptr;
					break;
				case FieldType::Uint32:
				case FieldType::Enum_Uint32:
					out << *(uint32*)ptr;
					break;
				case FieldType::Uint64:
				case FieldType::Enum_Uint64:
					out << *(uint64*)ptr;
					break;
				case FieldType::Float32:
					out << *(float*)ptr;
					break;
				case FieldType::Float64:
					out << *(double*)ptr;
					break;
				case FieldType::Vec2:
					out << *(glm::vec2*)ptr;
					break;
				case FieldType::Vec3:
					out << *(glm::vec3*)ptr;
					break;
				case FieldType::Vec4:
					out << *(glm::vec4*)ptr;
					break;
				case FieldType::String:
					out << *(std::string*)ptr;
					break;
				case FieldType::Asset:
					out << (*(Ref<Asset>*)ptr)->ID.str();
					break;
				case FieldType::Entity:
					out << ((Entity*)ptr)->GetID().str();
					break;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpeakerComponent>())
		{
			const auto& speaker = entity.GetComponent<SpeakerComponent>();
			out << YAML::Key << "SpeakerComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "AudioClip" << YAML::Value << speaker.AudioClip->ID.str();
			out << YAML::Key << "Volume" << YAML::Value << speaker.Volume;
			out << YAML::Key << "Pitch" << YAML::Value << speaker.Pitch;
			out << YAML::Key << "RollOffFactor" << YAML::Value << speaker.RollOffFactor;
			out << YAML::Key << "RefDistance" << YAML::Value << speaker.RefDistance;
			out << YAML::Key << "MaxDistance" << YAML::Value << speaker.MaxDistance;
			out << YAML::Key << "Looping" << YAML::Value << speaker.Looping;
			out << YAML::Key << "PlayOnStart" << YAML::Value << speaker.PlayOnStart;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<ParticleSystemComponent>())
		{
			const auto& psc = entity.GetComponent<ParticleSystemComponent>();
			out << YAML::Key << "ParticleSystemComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Position" << YAML::Value << psc.Props.Position;
			out << YAML::Key << "Velocity" << YAML::Value << psc.Props.Velocity;
			out << YAML::Key << "VelocityVariation" << YAML::Value << psc.Props.VelocityVariation;
			out << YAML::Key << "ColorBegin" << YAML::Value << psc.Props.ColorBegin;
			out << YAML::Key << "ColorEnd" << YAML::Value << psc.Props.ColorEnd;
			out << YAML::Key << "SizeBegin" << YAML::Value << psc.Props.SizeBegin;
			out << YAML::Key << "SizeEnd" << YAML::Value << psc.Props.SizeEnd;
			out << YAML::Key << "Rotation" << YAML::Value << psc.Props.Rotation;
			out << YAML::Key << "AngularVelocity" << YAML::Value << psc.Props.AngularVelocity;
			out << YAML::Key << "AngularVelocityVariation" << YAML::Value << psc.Props.AngularVelocityVariation;
			out << YAML::Key << "Duration" << YAML::Value << psc.Props.Duration;
			out << YAML::Key << "LifeTime" << YAML::Value << psc.Props.LifeTime;
			out << YAML::Key << "EmitionRate" << YAML::Value << psc.Props.EmitionRate;
			out << YAML::Key << "MaxParticles" << YAML::Value << psc.Props.MaxParticles;
			out << YAML::Key << "Looping" << YAML::Value << psc.Props.Looping;
			out << YAML::Key << "PlayOnStart" << YAML::Value << psc.PlayOnStart;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<AnimationComponent>())
		{
			const auto& ac = entity.GetComponent<AnimationComponent>();
			out << YAML::Key << "AnimationComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Animation" << YAML::Value << ac.Animation->ID.str();
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

}

