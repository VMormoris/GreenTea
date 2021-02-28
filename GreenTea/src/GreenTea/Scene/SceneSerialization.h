#ifndef _SCENE_SERIALIZATION
#define _SCENE_SERIALIZATION

#include "Components.h"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

#include <entt.hpp>

namespace GTE {

	template<typename BinaryArchive>
	void serialize(BinaryArchive& archive, std::string& str) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, std::string& str)
	{
		size_t size = str.size();
		archive(size);
		archive(cereal::binary_data(str.data(), size));
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, std::string& str)
	{
		size_t size;
		archive(size);
		//TODO: Consider using alloca() instead
		char* buffer = new char[size + 1];
		archive(cereal::binary_data(buffer, size));
		buffer[size] = '\0';
		str = std::string(buffer);
		delete[] buffer;
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, TagComponent& tag)
	{
		serialize(archive, tag.Tag);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, TagComponent& tag)
	{
		archive(cereal::make_nvp("Tag", tag.Tag));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, TagComponent& tag) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, TagComponent& tag)
	{
		BinSerialize(archive, tag);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, TagComponent& tag)
	{
		BinSerialize(archive, tag);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, TagComponent& tag)
	{
		JSONSerialize(archive, tag);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, TagComponent& tag)
	{
		JSONSerialize(archive, tag);
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, Transform2DComponent& transform)
	{
		archive(transform.Position.x);
		archive(transform.Position.y);
		archive(transform.Position.z);

		archive(transform.Scale.x);
		archive(transform.Scale.y);

		archive(transform.Rotation);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, Transform2DComponent& transform)
	{
		archive.setNextName("Position");
		archive.startNode();
		archive(cereal::make_nvp("x", transform.Position.x));
		archive(cereal::make_nvp("y", transform.Position.y));
		archive(cereal::make_nvp("z", transform.Position.z));
		archive.finishNode();

		archive.setNextName("Scale");
		archive.startNode();
		archive(cereal::make_nvp("x", transform.Scale.x));
		archive(cereal::make_nvp("y", transform.Scale.y));
		archive.finishNode();

		archive(cereal::make_nvp("angle", transform.Rotation));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, Transform2DComponent& transform) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, Transform2DComponent& transform)
	{
		BinSerialize(archive, transform);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, Transform2DComponent& transform)
	{
		BinSerialize(archive, transform);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, Transform2DComponent& transform)
	{
		JSONSerialize(archive, transform);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, Transform2DComponent& transform)
	{
		JSONSerialize(archive, transform);
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, Renderable2DComponent& renderable)
	{

		archive(renderable.Color.r);
		archive(renderable.Color.g);
		archive(renderable.Color.b);
		archive(renderable.Color.a);

		serialize(archive, renderable.Filepath);

		archive(renderable.FlipX);
		archive(renderable.FlipY);
		
		archive(renderable.TextCoords.BottomLeft.x); archive(renderable.TextCoords.BottomLeft.y);
		archive(renderable.TextCoords.BottomRight.x); archive(renderable.TextCoords.BottomRight.y);
		archive(renderable.TextCoords.TopRight.x); archive(renderable.TextCoords.TopRight.y);
		archive(renderable.TextCoords.TopLeft.x); archive(renderable.TextCoords.TopLeft.y);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, Renderable2DComponent& renderable)
	{
		archive.setNextName("Color");
		archive.startNode();
		archive(cereal::make_nvp("r", renderable.Color.r));
		archive(cereal::make_nvp("g", renderable.Color.g));
		archive(cereal::make_nvp("b", renderable.Color.b));
		archive(cereal::make_nvp("a", renderable.Color.a));
		archive.finishNode();

		archive(cereal::make_nvp("Filepath", renderable.Filepath));

		archive.setNextName("Flip");
		archive.startNode();
		archive(cereal::make_nvp("x", renderable.FlipX));
		archive(cereal::make_nvp("y", renderable.FlipY));
		archive.finishNode();

		archive.setNextName("Texture Coordinates");
		archive.startNode();

			archive.setNextName("BottomLeft");
			archive.startNode();
				archive(cereal::make_nvp("x", renderable.TextCoords.BottomLeft.x));
				archive(cereal::make_nvp("y", renderable.TextCoords.BottomLeft.y));
			archive.finishNode();

			archive.setNextName("BottomRight");
			archive.startNode();
				archive(cereal::make_nvp("x", renderable.TextCoords.BottomRight.x));
				archive(cereal::make_nvp("y", renderable.TextCoords.BottomRight.y));
			archive.finishNode();

			archive.setNextName("BottomLeft");
			archive.startNode();
				archive(cereal::make_nvp("x", renderable.TextCoords.TopRight.x));
				archive(cereal::make_nvp("y", renderable.TextCoords.TopRight.y));
			archive.finishNode();

			archive.setNextName("BottomRight");
			archive.startNode();
				archive(cereal::make_nvp("x", renderable.TextCoords.TopLeft.x));
				archive(cereal::make_nvp("y", renderable.TextCoords.TopLeft.y));
			archive.finishNode();

		archive.finishNode();
	}

	template<typename Archive>
	inline void serialize(Archive& archive, Renderable2DComponent& renderable) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, Renderable2DComponent& renderable)
	{
		BinSerialize(archive, renderable);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, Renderable2DComponent& renderable)
	{
		BinSerialize(archive, renderable);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, Renderable2DComponent& renderable)
	{
		JSONSerialize(archive, renderable);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, Renderable2DComponent& renderable)
	{
		JSONSerialize(archive, renderable);
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, RigidBody2DComponent& rigidbody)
	{
		archive(rigidbody.Velocity.x);
		archive(rigidbody.Velocity.y);

		archive(rigidbody.Type);
		archive(rigidbody.AngularVelocity);
		archive(rigidbody.Mass);
		archive(rigidbody.GravityFactor);
		archive(rigidbody.FixedRotation);
		archive(rigidbody.Bullet);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, RigidBody2DComponent& rigidbody)
	{
		archive.setNextName("Velocity");
		archive.startNode();
		archive(cereal::make_nvp("x", rigidbody.Velocity.x));
		archive(cereal::make_nvp("y", rigidbody.Velocity.y));
		archive.finishNode();

		archive(cereal::make_nvp("Type", rigidbody.Type));
		archive(cereal::make_nvp("Angular Velocity", rigidbody.AngularVelocity));
		archive(cereal::make_nvp("Mass", rigidbody.Mass));
		archive(cereal::make_nvp("GravityFactor", rigidbody.GravityFactor));
		archive(cereal::make_nvp("FixedRotation", rigidbody.FixedRotation));
		archive(cereal::make_nvp("Bullet", rigidbody.Bullet));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, RigidBody2DComponent& rigidbody) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, RigidBody2DComponent& rigidbody)
	{
		BinSerialize(archive, rigidbody);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, RigidBody2DComponent& rigidbody)
	{
		BinSerialize(archive, rigidbody);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, RigidBody2DComponent& rigidbody)
	{
		JSONSerialize(archive, rigidbody);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, RigidBody2DComponent& rigidbody)
	{
		JSONSerialize(archive, rigidbody);
	}

	template<typename Archive>
	void serialize(Archive& archive, CircleColliderComponent& collider)
	{
		archive(cereal::make_nvp("Radius", collider.Radius));
		archive(cereal::make_nvp("Friction", collider.Friction));
		archive(cereal::make_nvp("Restitution", collider.Restitution));
	}

	template<typename BinaryArchive>
	void BinSerialze(BinaryArchive& archive, BoxColliderComponent& collider)
	{

		archive(collider.Scale.x);
		archive(collider.Scale.y);

		archive(collider.Friction);
		archive(collider.Restitution);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, BoxColliderComponent& collider)
	{
		archive.setNextName("Scale");
		archive.startNode();
		archive(cereal::make_nvp("x", collider.Scale.x));
		archive(cereal::make_nvp("y", collider.Scale.y));
		archive.finishNode();

		archive(cereal::make_nvp("Friction", collider.Friction));
		archive(cereal::make_nvp("Restitution", collider.Restitution));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, BoxColliderComponent& collider) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, BoxColliderComponent& collider)
	{
		BinSerialze(archive, collider);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, BoxColliderComponent& collider)
	{
		BinSerialze(archive, collider);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, BoxColliderComponent& collider)
	{
		JSONSerialize(archive, collider);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, BoxColliderComponent& collider)
	{
		JSONSerialize(archive, collider);
	}

	template<typename Archive>
	void serialize(Archive& archive, OrthographicCameraComponent& ortho)
	{
		archive(cereal::make_nvp("Zoom Level", ortho.ZoomLevel));
		archive(cereal::make_nvp("Vertical Boundary", ortho.VerticalBoundary));
	}

	template<typename Archive>
	void serialize(Archive& archive, CameraComponent& cam)
	{
		archive(cereal::make_nvp("Aspect Ratio", cam.AspectRatio));
		archive(cereal::make_nvp("Primary", cam.Primary));
		archive(cereal::make_nvp("Fixed Aspect Ratio", cam.FixedAspectRatio));
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		archive(sceneProps.Gravity.x);
		archive(sceneProps.Gravity.y);
		archive(sceneProps.Rate);
		archive(sceneProps.VelocityIterations);
		archive(sceneProps.PositionIterations);

		archive(sceneProps.CamVelocity.x);
		archive(sceneProps.CamVelocity.y);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		archive.setNextName("g");
		archive.startNode();
		archive(cereal::make_nvp("x", sceneProps.Gravity.x));
		archive(cereal::make_nvp("y", sceneProps.Gravity.y));
		archive.finishNode();

		archive(cereal::make_nvp("Rate", sceneProps.Rate));
		archive(cereal::make_nvp("Velocity Iterations", sceneProps.VelocityIterations));
		archive(cereal::make_nvp("Position Iterations", sceneProps.PositionIterations));

		archive.setNextName("Camera's Velocity");
		archive.startNode();
		archive(cereal::make_nvp("x", sceneProps.CamVelocity.x));
		archive(cereal::make_nvp("y", sceneProps.CamVelocity.y));
		archive.finishNode();
	}

	template<typename Archive>
	inline void serialize(Archive& archive, ScenePropertiesComponent& sceneProps) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		BinSerialize(archive, sceneProps);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		BinSerialize(archive, sceneProps);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		JSONSerialize(archive, sceneProps);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		JSONSerialize(archive, sceneProps);
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, NativeScriptComponent& nScript)
	{
		archive(nScript.State);
		serialize(archive, nScript.ClassName);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, NativeScriptComponent& nScript)
	{
		archive(cereal::make_nvp("State", nScript.State));
		archive(cereal::make_nvp("Class Name", nScript.ClassName));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, NativeScriptComponent& nScript) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, NativeScriptComponent& nScript)
	{
		BinSerialize(archive, nScript);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, NativeScriptComponent& nScript)
	{
		BinSerialize(archive, nScript);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, NativeScriptComponent& nScript)
	{
		JSONSerialize(archive, nScript);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, NativeScriptComponent& nScript)
	{
		JSONSerialize(archive, nScript);
	}

	template<typename Archive>
	void serialize(Archive& archive, RelationshipComponent& relationship)
	{
		archive(cereal::make_nvp("Childrens", relationship.Childrens));
		archive(cereal::make_nvp("First Child", relationship.FirstChild));
		archive(cereal::make_nvp("Previous", relationship.Previous));
		archive(cereal::make_nvp("Next", relationship.Next));
		archive(cereal::make_nvp("Parent", relationship.Parent));
	}

	template<typename JSONArchive>
	size_t save(JSONArchive& archive, const entt::registry& reg, const entt::entity entity)
	{
		archive.startNode();
		archive(cereal::make_nvp("ID", entity));
		reg.visit(entity, [&](const auto component) {
			switch (component)
			{
			case entt::type_info<TagComponent>::id():
			{
				const auto& tag = reg.get<TagComponent>(entity);
				archive.setNextName("Tag Component");
				archive(tag);
				break;
			}
			case entt::type_info<Transform2DComponent>::id():
			{
				const auto& transform = reg.get<Transform2DComponent>(entity);
				archive.setNextName("Transform 2D Component");
				archive(transform);
				break;
			}
			case entt::type_info<Renderable2DComponent>::id():
			{
				const auto& renderable = reg.get<Renderable2DComponent>(entity);
				archive.setNextName("Renderable 2D Component");
				archive(renderable);
				break;
			}
			case entt::type_info<RigidBody2DComponent>::id():
			{
				const auto& rigidbody = reg.get<RigidBody2DComponent>(entity);
				archive.setNextName("RigidBody 2D Component");
				archive(rigidbody);
				break;
			}
			case entt::type_info<CircleColliderComponent>::id():
			{
				const auto& ccollider = reg.get<CircleColliderComponent>(entity);
				archive.setNextName("Circle Collider Component");
				archive(ccollider);
				break;
			}
			case entt::type_info<BoxColliderComponent>::id():
			{
				const auto& bcollider = reg.get<BoxColliderComponent>(entity);
				archive.setNextName("Box Collider Component");
				archive(bcollider);
				break;
			}
			case entt::type_info<NativeScriptComponent>::id():
			{
				const auto& nScript = reg.get<NativeScriptComponent>(entity);
				archive.setNextName("Native Script Component");
				archive(nScript);
				break;
			}
			case entt::type_info<OrthographicCameraComponent>::id():
			{
				const auto& camProp = reg.get<OrthographicCameraComponent>(entity);
				archive.setNextName("Orthographic Camera Component");
				archive(camProp);
				break;
			}
			case entt::type_info<CameraComponent>::id():
			{
				const auto& cam = reg.get<CameraComponent>(entity);
				archive.setNextName("Camera Component");
				archive(cam);
				break;
			}
			case entt::type_info<ScenePropertiesComponent>::id():
			{
				const auto& sceneProp = reg.get<ScenePropertiesComponent>(entity);
				archive.setNextName("Scene Properties Component");
				archive(sceneProp);
				break;
			}
			default:
				break;
			}
		});

		const auto& rel = reg.get<RelationshipComponent>(entity);
		archive(cereal::make_nvp("Childrens", rel.Childrens));
		archive.setNextName("Entities");
		archive.startNode();
		archive.makeArray();
		entt::entity child = rel.FirstChild;
		for (size_t i = 0; i < rel.Childrens; i++)
		{
			save(archive, reg, child);
			const auto& crel = reg.get<RelationshipComponent>(child);
			child = crel.Next;
		}
		archive.finishNode();
		archive.finishNode();
		return rel.Childrens + 1;
	}

	template<typename JSONArchive>
	void save(JSONArchive& archive, const entt::registry& reg)
	{
		size_t entities = reg.alive();
		archive(cereal::make_nvp("Total Entities", entities));
		archive.setNextName("Entities");
		archive.startNode();
		archive.makeArray();
		size_t written = 0;
		const entt::entity* entity = reg.data();
		while (written < entities)
		{
			while (!reg.valid(*entity)) entity++;
			if (!reg.has<RelationshipComponent>(*entity))
				written += save(archive, reg, *entity);
			else if (reg.get<RelationshipComponent>(*entity).Parent == entt::null)
				written += save(archive, reg, *entity);
			entity++;
		}
		archive.finishNode();
	}

	template<typename JSONArchive>
	size_t loadentity(JSONArchive& archive, entt::registry& reg, entt::entity& entity)
	{
		archive.startNode();
		entt::entity entt;
		archive(cereal::make_nvp("ID", entt));
		entt = reg.create(entt);

		const char* compname = nullptr;
		while ((compname = archive.getNodeName()) != nullptr)
		{
			std::string component(compname);
			if (component.compare("Childrens") == 0) break;
			if (component.compare("Tag Component") == 0)
			{
				TagComponent tag;
				archive(tag);
				reg.emplace<TagComponent>(entt, tag);
			}
			else if (component.compare("Transform 2D Component") == 0)
			{
				Transform2DComponent tranform;
				archive(tranform);
				reg.emplace<Transform2DComponent>(entt, tranform);
			}
			else if (component.compare("Renderable 2D Component") == 0)
			{
				Renderable2DComponent renderable;
				archive(renderable);
				reg.emplace<Renderable2DComponent>(entt, renderable);
			}
			else if (component.compare("RigidBody 2D Component") == 0)
			{
				RigidBody2DComponent rigidbody;
				archive(rigidbody);
				reg.emplace<RigidBody2DComponent>(entt, rigidbody);
			}
			else if (component.compare("Circle Collider Component") == 0)
			{
				CircleColliderComponent ccollider;
				archive(ccollider);
				reg.emplace<CircleColliderComponent>(entt, ccollider);
			}
			else if (component.compare("Box Collider Component") == 0)
			{
				BoxColliderComponent bcollider;
				archive(bcollider);
				reg.emplace<BoxColliderComponent>(entt, bcollider);
			}
			else if (component.compare("Native Script Component") == 0)
			{
				NativeScriptComponent nScript;
				archive(nScript);
				reg.emplace<NativeScriptComponent>(entt, nScript);
			}
			else if (component.compare("Orthographic Camera Component") == 0)
			{
				OrthographicCameraComponent ortho;
				archive(ortho);
				reg.emplace<OrthographicCameraComponent>(entt, ortho);
			}
			else if (component.compare("Camera Component") == 0)
			{
				CameraComponent cam;
				archive(cam);
				reg.emplace<CameraComponent>(entt, cam);
			}
			else if (component.compare("Scene Properties Component") == 0)
			{
				ScenePropertiesComponent sceneProp;
				archive(sceneProp);
				reg.emplace<ScenePropertiesComponent>(entt, sceneProp);
			}
		}
		size_t size, childNum = 0;
		archive(cereal::make_nvp("Childrens", size));
		std::vector<entt::entity> childrens;
		childrens.reserve(size);
		archive.getNodeName();//Entities
		archive.startNode();
		for (size_t i = 0; i < size; i++)
		{
			entt::entity child = entt::null;
			childNum += loadentity(archive, reg, child);
			childrens.push_back(child);
		}
		archive.finishNode();
		auto& rel = reg.emplace<RelationshipComponent>(entt);
		rel.Childrens = size;
		if (size > 0)
		{
			rel.FirstChild = childrens[0];
			auto& crel = reg.get<RelationshipComponent>(childrens[0]);
			crel.Parent = entt;
			for (size_t i = 1; i < size; i++)
			{
				const entt::entity previous = childrens[i - 1];
				const entt::entity current = childrens[i];
				auto& currRel = reg.get<RelationshipComponent>(current);
				auto& prevRel = reg.get<RelationshipComponent>(previous);
				currRel.Previous = previous;
				prevRel.Next = current;
				currRel.Parent = entt;
				prevRel.Parent = entt;
			}
		}
		archive.finishNode();
		entity = entt;
		return 1 + childNum;
	}

	template<typename JSONArchive>
	void load(JSONArchive& archive, entt::registry& reg)
	{
		size_t entities;
		archive(cereal::make_nvp("Total Entities", entities));
		archive.getNodeName();//Entities
		archive.startNode();
		size_t readen = 0;
		while (readen < entities)
		{
			entt::entity entity = entt::null;
			readen += loadentity(archive, reg, entity);
		}
		archive.finishNode();
	}

}


#endif