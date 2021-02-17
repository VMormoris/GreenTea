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
	void BinSerialize(BinaryArchive& archive, TransformComponent& transform)
	{
		archive(transform.Position.x);
		archive(transform.Position.y);
		archive(transform.Position.z);


		archive(transform.Scale.x);
		archive(transform.Scale.y);
		archive(transform.Scale.z);

		archive(transform.Rotation.x);
		archive(transform.Rotation.y);
		archive(transform.Rotation.z);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, TransformComponent& transform)
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
		archive(cereal::make_nvp("z", transform.Scale.z));
		archive.finishNode();

		archive.setNextName("Rotation");
		archive.startNode();
		archive(cereal::make_nvp("x", transform.Rotation.x));
		archive(cereal::make_nvp("y", transform.Rotation.y));
		archive(cereal::make_nvp("z", transform.Rotation.z));
		archive.finishNode();
	}

	template<typename Archive>
	inline void serialize(Archive& archive, TransformComponent& transform) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, TransformComponent& transform)
	{
		BinSerialize(archive, transform);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, TransformComponent& transform)
	{
		BinSerialize(archive, transform);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, TransformComponent& transform)
	{
		JSONSerialize(archive, transform);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, TransformComponent& transform)
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
	void BinSerialize(BinaryArchive& archive, MeshComponent& mesh)
	{
		serialize(archive, mesh.Filepath);
		archive(mesh.MaterialIndex);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, MeshComponent& mesh)
	{
		archive(cereal::make_nvp("Mesh", mesh.Filepath));
		archive(cereal::make_nvp("Material Index: ", mesh.MaterialIndex));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, MeshComponent& mesh) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, MeshComponent& mesh)
	{
		BinSerialize(archive, mesh);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, MeshComponent& mesh)
	{
		BinSerialize(archive, mesh);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, MeshComponent& mesh)
	{
		JSONSerialize(archive, mesh);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, MeshComponent& mesh)
	{
		JSONSerialize(archive, mesh);
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, PerspectiveCameraComponent& persp)
	{
		archive(persp.Target.x);
		archive(persp.Target.y);
		archive(persp.Target.z);

		archive(persp.UpVector.x);
		archive(persp.UpVector.y);
		archive(persp.UpVector.z);

		archive(persp.FoV);
		archive(persp.Near);
		archive(persp.Far);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, PerspectiveCameraComponent& persp)
	{
		archive.setNextName("Target");
		archive.startNode();
		archive(cereal::make_nvp("x", persp.Target.x));
		archive(cereal::make_nvp("y", persp.Target.y));
		archive(cereal::make_nvp("z", persp.Target.z));
		archive.finishNode();

		archive.setNextName("Up Vector");
		archive.startNode();
		archive(cereal::make_nvp("x", persp.UpVector.x));
		archive(cereal::make_nvp("y", persp.UpVector.y));
		archive(cereal::make_nvp("z", persp.UpVector.z));
		archive.finishNode();

		archive(cereal::make_nvp("FoV", persp.FoV));
		archive(cereal::make_nvp("Near", persp.Near));
		archive(cereal::make_nvp("Far", persp.Far));
	}
	
	template<typename Archive>
	inline void serialize(Archive& archive, PerspectiveCameraComponent& persp) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, PerspectiveCameraComponent& persp)
	{
		BinSerialize(archive, persp);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, PerspectiveCameraComponent& persp)
	{
		BinSerialize(archive, persp);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, PerspectiveCameraComponent& persp)
	{
		JSONSerialize(archive, persp);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, PerspectiveCameraComponent& persp)
	{
		JSONSerialize(archive, persp);
	}

	template<typename Archive>
	void serialize(Archive& archive, CameraComponent& cam)
	{
		archive(cereal::make_nvp("Aspect Ratio", cam.AspectRatio));
		archive(cereal::make_nvp("Primary", cam.Primary));
		archive(cereal::make_nvp("Fixed Aspect Ratio", cam.FixedAspectRatio));
	}

	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, LightComponent& lc)
	{
		archive(lc.Target.x);
		archive(lc.Target.y);
		archive(lc.Target.z);

		archive(lc.Direction.x);
		archive(lc.Direction.y);
		archive(lc.Direction.z);

		archive(lc.Color.r);
		archive(lc.Color.g);
		archive(lc.Color.b);
		archive(lc.Color.a);

		archive(lc.Intensity);
		archive(lc.Umbra);
		archive(lc.Penumbra);
		archive(lc.Near);
		archive(lc.Far);
		archive(lc.ShadowMapBias);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, LightComponent& lc)
	{
		archive.setNextName("Target");
		archive.startNode();
		archive(cereal::make_nvp("x", lc.Target.x));
		archive(cereal::make_nvp("y", lc.Target.y));
		archive(cereal::make_nvp("z", lc.Target.z));
		archive.finishNode();

		archive.setNextName("Direction");
		archive.startNode();
		archive(cereal::make_nvp("x", lc.Direction.x));
		archive(cereal::make_nvp("y", lc.Direction.y));
		archive(cereal::make_nvp("z", lc.Direction.z));
		archive.finishNode();

		archive.setNextName("Color");
		archive.startNode();
		archive(cereal::make_nvp("r", lc.Color.r));
		archive(cereal::make_nvp("g", lc.Color.g));
		archive(cereal::make_nvp("b", lc.Color.b));
		archive(cereal::make_nvp("a", lc.Color.a));
		archive.finishNode();

		archive(cereal::make_nvp("Intensity", lc.Intensity));
		archive(cereal::make_nvp("Umbra", lc.Umbra));
		archive(cereal::make_nvp("Penumbra", lc.Penumbra));
		archive(cereal::make_nvp("Near", lc.Near));
		archive(cereal::make_nvp("Far", lc.Far));
		archive(cereal::make_nvp("Shadow Bias", lc.ShadowMapBias));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, LightComponent& lc) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, LightComponent& lc)
	{
		BinSerialize(archive, lc);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, LightComponent& lc)
	{
		BinSerialize(archive, lc);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, LightComponent& lc)
	{
		JSONSerialize(archive, lc);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, LightComponent& lc)
	{
		JSONSerialize(archive, lc);
	}


	template<typename BinaryArchive>
	void BinSerialize(BinaryArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		archive(sceneProps.ShadowmapResolution.x);
		archive(sceneProps.ShadowmapResolution.y);

		archive(sceneProps.CamVelocity);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, ScenePropertiesComponent& sceneProps)
	{
		archive.setNextName("Shadowmap Resolution");
		archive.startNode();
		archive(cereal::make_nvp("x", sceneProps.ShadowmapResolution.x));
		archive(cereal::make_nvp("y", sceneProps.ShadowmapResolution.y));
		archive.finishNode();

		archive(cereal::make_nvp("Camera's Velocity", sceneProps.CamVelocity));
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
	void BinSerialize(BinaryArchive& archive, EnviromentComponent& env)
	{
		serialize(archive, env.SkyboxFilepath);
	}

	template<typename JSONArchive>
	void JSONSerialize(JSONArchive& archive, EnviromentComponent& env)
	{
		archive(cereal::make_nvp("Skybox Filepath", env.SkyboxFilepath));
	}

	template<typename Archive>
	inline void serialize(Archive& archive, EnviromentComponent& env) { static_assert(false); }

	template<>
	inline void serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive& archive, EnviromentComponent& env)
	{
		BinSerialize(archive, env);
	}

	template<>
	inline void serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive& archive, EnviromentComponent& env)
	{
		BinSerialize(archive, env);
	}

	template<>
	inline void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive& archive, EnviromentComponent& env)
	{
		JSONSerialize(archive, env);
	}

	template<>
	inline void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive& archive, EnviromentComponent& env)
	{
		JSONSerialize(archive, env);
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
			case entt::type_info<TransformComponent>::id():
			{
				const auto& transform = reg.get<TransformComponent>(entity);
				archive.setNextName("Transform Component");
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
			case entt::type_info<MeshComponent>::id():
			{
				const auto& mc = reg.get<MeshComponent>(entity);
				archive.setNextName("Mesh Component");
				archive(mc);
				break;
			}
			case entt::type_info<NativeScriptComponent>::id():
			{
				const auto& nScript = reg.get<NativeScriptComponent>(entity);
				archive.setNextName("Native Script Component");
				archive(nScript);
				break;
			}
			case entt::type_info<PerspectiveCameraComponent>::id():
			{
				const auto& camProp = reg.get<PerspectiveCameraComponent>(entity);
				archive.setNextName("Perspective Camera Component");
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
			case entt::type_info<LightComponent>::id():
			{
				const auto& lc = reg.get<LightComponent>(entity);
				archive.setNextName("Light Component");
				archive(lc);
				break;
			}
			case entt::type_info<ScenePropertiesComponent>::id():
			{
				const auto& sceneProp = reg.get<ScenePropertiesComponent>(entity);
				archive.setNextName("Scene Properties Component");
				archive(sceneProp);
				break;
			}
			case entt::type_info<EnviromentComponent>::id():
			{
				const auto& env = reg.get<EnviromentComponent>(entity);
				archive.setNextName("Enviroment Component");
				archive(env);
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
			else if (component.compare("Transform Component") == 0)
			{
				TransformComponent tranform;
				archive(tranform);
				reg.emplace<TransformComponent>(entt, tranform);
			}
			else if (component.compare("Renderable 2D Component") == 0)
			{
				Renderable2DComponent renderable;
				archive(renderable);
				reg.emplace<Renderable2DComponent>(entt, renderable);
			}
			else if (component.compare("Mesh Component") == 0)
			{
				MeshComponent mc;
				archive(mc);
				reg.emplace<MeshComponent>(entt, mc);
			}
			else if (component.compare("Native Script Component") == 0)
			{
				NativeScriptComponent nScript;
				archive(nScript);
				reg.emplace<NativeScriptComponent>(entt, nScript);
			}
			else if (component.compare("Perspective Camera Component") == 0)
			{
				PerspectiveCameraComponent persp;
				archive(persp);
				reg.emplace<PerspectiveCameraComponent>(entt, persp);
			}
			else if (component.compare("Camera Component") == 0)
			{
				CameraComponent cam;
				archive(cam);
				reg.emplace<CameraComponent>(entt, cam);
			}
			else if (component.compare("Light Component") == 0)
			{
				LightComponent lc;
				archive(lc);
				reg.emplace<LightComponent>(entt, lc);
			}
			else if (component.compare("Scene Properties Component") == 0)
			{
				ScenePropertiesComponent sceneProp;
				archive(sceneProp);
				reg.emplace<ScenePropertiesComponent>(entt, sceneProp);
			}
			else if (component.compare("Enviroment Component") == 0)
			{
				EnviromentComponent env;
				archive(env);
				reg.emplace<EnviromentComponent>(entt, env);
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