#pragma once

#include "Engine.h"
#include <glm.hpp>

#include <yaml-cpp/emitter.h>

namespace gte::math {

	ENGINE_API bool DecomposeTransform(const glm::mat4& transform, glm::vec3& position, glm::vec3& scale, glm::vec3& rotation) noexcept;

	ENGINE_API YAML::Emitter& operator<<(YAML::Emitter& emitter, const glm::vec2& vec) noexcept;
	ENGINE_API YAML::Emitter& operator<<(YAML::Emitter& emitter, const glm::vec3& vec) noexcept;
	ENGINE_API YAML::Emitter& operator<<(YAML::Emitter& emitter, const glm::vec4& vec) noexcept;


	template<typename T>
	float SquareDistance(const T& lhs, const T& rhs) noexcept { static_assert("Not implemented"); }
}

#include "Math.hpp"