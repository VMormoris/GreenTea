#pragma once

#include "EngineCore.h"

#include <glm.hpp>

namespace GTE::Math {

	ENGINE_API bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

}