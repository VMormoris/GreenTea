// Modified from: https://github.com/StudioCherno/Walnut/blob/ad2f2939686ea5f891c72d9453220ea0e787744a/Walnut/Platform/GUI/Walnut/UI/UI.cpp
#pragma once
#ifndef GT_DIST

#include <Engine/Core/Engine.h>

#include <glm.hpp>
#include <imgui_internal.h>

namespace gte::gui {
	
	ENGINE_API void ShiftCursor(const glm::vec2& offset);
	ENGINE_API void ShiftCursorX(float distance);
	ENGINE_API void ShiftCursorY(float distance);

	ENGINE_API ImRect GetItemRect(void);
	ENGINE_API ImRect RectExpanded(const ImRect& rect, const glm::vec2& size);
	ENGINE_API ImRect RectOffset(const ImRect& rect, const glm::vec2& offset);

	ENGINE_API void DrawButtonImage(const ImTextureID id,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax);
	ENGINE_API void DrawButtonImage(const ImTextureID id,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle);
	ENGINE_API void DrawButtonImage(const ImTextureID id,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed);

	ENGINE_API void RenderWindowOuterBorders(ImGuiWindow* window);

	// Exposed resize behavior for native OS windows
	ENGINE_API bool UpdateWindowManualResize(ImGuiWindow* window, ImVec2& newSize, ImVec2& newPosition);

	ENGINE_API bool BeginMenubar(const ImRect& barRectangle);
	ENGINE_API bool BeginMenubar(const ImRect& barRectangle);
	ENGINE_API void EndMenubar(void);
}

#endif