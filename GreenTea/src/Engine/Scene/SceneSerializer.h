#pragma once

#include "Scene.h"

//Forward Decleration(s)
namespace YAML { class Emitter; }

namespace gte::internal {

	class ENGINE_API SceneSerializer {
	public:
		SceneSerializer(Scene* scene);

		void Serialize(const std::string& filepath);
		void Deserialize(const std::string& filepath);

	private:
	
		void SerializeEntity(YAML::Emitter& out, gte::Entity entity);
	
	private:
		Scene* mScene = nullptr;
	};
}