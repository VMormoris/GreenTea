#pragma once

#include "Math.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.r);
			node.push_back(rhs.g);
			node.push_back(rhs.b);
			node.push_back(rhs.a);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;
			rhs.r = node[0].as<float>();
			rhs.g = node[1].as<float>();
			rhs.b = node[2].as<float>();
			rhs.a = node[3].as<float>();
			return true;
		}
	};
}

namespace gte::math {

	template<>
	inline float SquareDistance<>(const glm::vec2& lhs, const glm::vec2& rhs) noexcept
	{
		let x = lhs.x - rhs.x;
		let y = lhs.y - rhs.y;
		return x * x + y * y;
	}

	template<>
	inline float SquareDistance<>(const glm::vec3& lhs, const glm::vec3& rhs) noexcept
	{
		let x = lhs.x - rhs.x;
		let y = lhs.y - rhs.y;
		let z = lhs.z - rhs.z;
		return x * x + y * y + z * z;
	}

	template<>
	inline float SquareDistance<>(const glm::vec4& lhs, const glm::vec4& rhs) noexcept
	{
		let x = lhs.x - rhs.x;
		let y = lhs.y - rhs.y;
		let z = lhs.z - rhs.z;
		let w = lhs.w - rhs.w;
		return x * x + y * y + z * z + w * w;
	}

}