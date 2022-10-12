#pragma once

#include <Engine/Core/Engine.h>

#include <yaml-cpp/node/node.h>

namespace gte {

	/**
	* @brief Wrapper over YAML::Node that holds Component(s)
	*/
	class ENGINE_API Prefab {
	public:
		Prefab(void) = default;
		Prefab(const YAML::Node& node) { mNode = node; }

		YAML::Node& GetNode(void) noexcept { return mNode; }
		const YAML::Node& GetNode(void) const noexcept { return mNode; }
	
	private:
		YAML::Node mNode;
	};

}