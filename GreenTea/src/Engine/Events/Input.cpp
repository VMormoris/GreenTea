#include "Input.h"

#include <Engine/Core/Context.h>
#include <Engine/NativeScripting/ScriptableEntity.h>

namespace gte {

	float Input::TimeSinceStart(void) { return internal::GetContext()->GlobalTime; }

	Entity Input::GetHoveredEntity(void)
	{
		using namespace internal;
		uint32 x, y;
		GetMousePos(x, y);
		x -= static_cast<uint32>(internal::GetContext()->ViewportOffset.x);
		y -= static_cast<uint32>(internal::GetContext()->ViewportOffset.y);
		y = static_cast<uint32>(internal::GetContext()->ViewportSize.y) - y;

		entt::entity enttID = entt::null;
		GPU::PixelBuffer* pbo = GetContext()->PixelBufferObject;
		if (!pbo)
			return {};
		pbo->GetPixel(0, static_cast<int32>(x), static_cast<int32>(y), &enttID);
		Entity entity = { enttID, GetContext()->ActiveScene };
		return entity ? entity : Entity{};
		return {};
	}

	bool Input::IsHovered(Entity entity) { return entity == GetHoveredEntity(); }

	bool Input::IsHovered(ScriptableEntity* entity)
	{
		if (Entity hovered = GetHoveredEntity())
		{
			if (!hovered.HasComponent<IDComponent>())
				return false;
			uuid enttUUID = entity->GetComponent<IDComponent>();
			uuid hoveredUUID = hovered.GetComponent<IDComponent>();
			return enttUUID == hoveredUUID;
		}
		return false;
	}

}