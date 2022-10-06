#pragma once

#include <Engine/Assets/NativeScript.h>
#include <Engine/GPU/Texture.h>
#include <Engine/NativeScripting/ScriptingEngine.h>
#include <Engine/Scene/Entity.h>

#include <glm.hpp>
#include <IconsForkAwesome.h>
#include <filesystem>


namespace gte::gui {

	template<typename T, typename Func>
	void DrawComponent(const char* icon, const char* name, Entity entity, Func&& func)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto IconsFont = io.Fonts->Fonts[3];
		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow;//| ImGuiTreeNodeFlags_SpanAvailWidth
		
		auto& component = entity.GetComponent<T>();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		//float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		ImGui::PushFont(IconsFont);
		const bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, icon);
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::Text(name);
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 24.0f);
		ImGui::PushFont(IconsFont);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		const bool remove = ImGui::Button(ICON_FK_TRASH);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopFont();
		ImGui::PopStyleVar();

		if (open)
		{
			func(component);
			ImGui::TreePop();
			ImGui::Separator();
		}

		if (remove)
			entity.RemoveComponent<T>();
	}

	/**
	* @brief Basically an input text with the classic icon used when searching.
	* @param label Element's label will be used as an id
	* @param text Text that has already been typped. If empty a hint "Search..." is used
	* @param length Maximum number of charaters that are allowed for the input
	* @return Whether input text has changed or not
	*/
	ENGINE_API bool DrawSearchbar(const char* label, std::string& text, size_t length, float width = 160.0f);

	/**
	* @brief Helper function for rendering as text but edit with double click
	* @param text Current text to be displayed/eddited
	* @param length Max size that the can have
	* @param editing Whether we are on editing mode or not
	* @return Whether the text has change or not
	*/
	ENGINE_API bool DrawEditText(std::string& text, size_t length, bool& editing);

	ENGINE_API bool DrawBreadcrumb(const char* label, std::filesystem::path& path);

	ENGINE_API bool DrawMenuItem(const char* icon, const char* item, const char* shortcut, const char* biggest = "");

	struct ENGINE_API UISettings {
		float ColumnWidth = 136.0f;
		float ResetValue = 0.0f;
		
		union {
			size_t Length;
			int64 MinInt = 0;
			uint64 MinUint;
			double MinFloat;
		};

		union {
			int64 MaxInt = 0;
			uint64 MaxUint;
			double MaxFloat;
		};

		float Speed = 0.1f;

		std::array<bool, 4> Enabled = { true, true, true, true };

		UISettings(void) = default;
		UISettings(float speed)
			: Speed(speed) {}
		UISettings(const UISettings&) = default;
	};

	ENGINE_API bool DrawBoolControl(const char* label, bool& value, const UISettings& settings);

	ENGINE_API bool DrawCharControl(const char* label, char& value, const UISettings& settings);

	ENGINE_API bool DrawInt16Control(const char* label, int16& value, const UISettings& settings);

	ENGINE_API bool DrawInt32Control(const char* label, int32& value, const UISettings& settings);

	ENGINE_API bool DrawInt64Control(const char* label, int64& value, const UISettings& settings);

	ENGINE_API bool DrawByteControl(const char* label, byte& value, const UISettings& settings);

	ENGINE_API bool DrawUint16Control(const char* label, uint16& value, const UISettings& settings);

	ENGINE_API bool DrawUint32Control(const char* label, uint32& value, const UISettings& settings);

	ENGINE_API bool DrawUint64Control(const char* label, uint64& value, const UISettings& settings);

	ENGINE_API bool DrawFloatControl(const char* label, float& value, const UISettings& settings);

	ENGINE_API bool DrawDoubleControl(const char* label, double& value, const UISettings& settings);

	ENGINE_API bool DrawStringControl(const char* label, std::string& value, const UISettings& settings);

	ENGINE_API bool DrawVec2Control(const char* label, glm::vec2& value, const UISettings& settings);

	ENGINE_API bool DrawVec3Control(const char* label, glm::vec3& value, const UISettings& settings);

	ENGINE_API bool DrawColorPicker(const char* label, glm::vec4& value, const UISettings& settings);

	ENGINE_API bool DrawComboControl(const char* label,  int32& index, const char* const* options, int32 size, const UISettings& settings);

	ENGINE_API bool DrawEnumControl(const char* label, const std::string& type_name, internal::Enum::Value& value, internal::FieldType type, const UISettings& settings);

	ENGINE_API bool DrawTextureCoordinates(TextureCoordinates& coords, uint32 width, uint32 height, const UISettings& settings);

	ENGINE_API bool DrawAssetControl(const char* label, uuid& id, const char* filetype, const UISettings& settings);

	ENGINE_API bool DrawEntityControl(const char* label, uuid& id, const UISettings& settings);

	ENGINE_API bool DrawFieldControl(const internal::FieldSpecification& spec, void* buffer);
}