#pragma once

#include <Engine/Core/Engine.h>

#include <entt.hpp>
#include <glm.hpp>

namespace gte::internal {

	ENGINE_API void RenderScene(entt::registry* reg, const glm::mat4& eyeMatrix);

}