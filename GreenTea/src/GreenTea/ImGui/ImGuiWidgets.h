#ifndef IM_GUI_WIDGETS
#define IM_GUI_WIDGETS

#include <GreenTea/Scene/Entity.h>

#include <GreenTea/GPU/Texture.h>

#include <GreenTea/Assets/Material.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace GTE {


	struct ENGINE_API UISettings {
		float ResetValue = 0.0f;
		float ColumnWidth = 100.0f;
		glm::vec2 Clamp = glm::vec2(0.0f);
		float Speed = 0.1f;
		std::array<bool, 4> Disabled = { false, false, false, false };
	};

	template<typename T, typename Func>
	void DrawComponent(const char* name, Entity entity, Func function)
	{
		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;//| ImGuiTreeNodeFlags_SpanAvailWidth
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name);
			ImGui::PopStyleVar();
			
			bool remove = false;
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete Component"))
					remove = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				function(component);
				ImGui::TreePop();
				ImGui::Separator();
			}

			if (remove)
				entity.RemoveComponent<T>();
		}
	}

	ENGINE_API void DrawComponentAdder(Entity entity);

	/**
	* @brief Helper function for Controling a Vec4 with ImGui
	*/
	ENGINE_API void DrawColorPicker(const char* label, glm::vec4& value, const UISettings& settings);

	/**
	* @brief Helper function for Controling a Vec3 with ImGui
	*/
	ENGINE_API bool DrawVec3Control(const char* label, glm::vec3& value, const UISettings& settings);

	/**
	* @brief Helper function for Controling a Vec2 with ImGui
	*/
	ENGINE_API bool DrawVec2Control(const char* label, glm::vec2& value, const UISettings& settings);

	ENGINE_API bool DrawFloatControl(const char* label, float& value, const UISettings& settings);

	ENGINE_API bool DrawIntControl(const char* label, int& value, const UISettings& settings);

	ENGINE_API void DrawComboControl(const char* label, int32& index, const char* const*  selection, int32 size, const UISettings& settings);

	ENGINE_API void DrawTextInput(const char* label, std::string& text, size_t length, const UISettings& settings);

	ENGINE_API bool DrawMenuItem(const char* icon, const char* item, const char* shortcut);

	ENGINE_API bool DrawFilePicker(const char* label, std::string& text, const char* extension, const UISettings& settings);

	ENGINE_API bool DrawIVec2Control(const char* label, glm::ivec2& value, const UISettings& settings);

	ENGINE_API bool DrawTextureCoordinates(TextureCoordinates& textCoords, uint32 width, uint32 height, const UISettings& settings);

	ENGINE_API bool DrawCheckboxControl(const char* label, bool& value, const UISettings& settings);

	ENGINE_API bool DrawMaterialControl(const char* label, Material& mat, const UISettings& settings);



}

#endif
