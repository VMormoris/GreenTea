#include "ImGuiWidgets.h"

#include <GreenTea/Core/FileDialog.h>

#include <GreenTea/Scene/Components.h>

namespace GTE {

	void DrawComponentAdder(Entity entity)
	{
		static constexpr char* components[] = { "Transform2D", "Mesh", "Camera", "Light", "NativeScript" };
		static int32 current = 0;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize{ 0.0f, lineHeight};

		const float width = ImGui::GetContentRegionAvailWidth() - 120.0f;
		ImGui::PushID("Component Adder");
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		ImGui::PushItemWidth(width);
		ImGui::Combo("##Components", &current, components, IM_ARRAYSIZE(components));
		ImGui::PopItemWidth();
		ImGui::PopStyleVar(1);

		
		ImGui::SameLine();

		ImGui::PushFont(boldFont);
		const bool pressed = ImGui::Button("Add Component", buttonSize);
		ImGui::PopFont();
		
		ImGui::PopID();
		if (pressed)
		{
			switch (current) {
			case 0://Transform 2D Component
				if (!entity.HasComponent<TransformComponent>()) entity.AddComponent<TransformComponent>();
				break;
			case 1://Mesh Component
				if (!entity.HasComponent<MeshComponent>())
				{
					if (!entity.HasComponent<TransformComponent>()) { GTE_WARN_LOG("Mesh without Transform will be ignored!"); }
					entity.AddComponent<MeshComponent>();
				}
				break;
			case 2://Camera Component
				if (!entity.HasComponent<CameraComponent>())
				{
					if (!entity.HasComponent<TransformComponent>())
					{
						auto& tc = entity.AddComponent<TransformComponent>();
						tc.Position.z = -190.0f;
						tc.Position.y = 80.0f;
						GTE_INFO_LOG("Automatically created Transform 2D Component!");
					}
					entity.AddComponent<CameraComponent>();
					auto& ortho = entity.AddComponent<PerspectiveCameraComponent>();
					entity.UpdateMatrices();
				}
				break;
			case 3://LightComponent
				if (!entity.HasComponent<LightComponent>())
				{
					auto& lc = entity.AddComponent<LightComponent>();
					lc.Direction = { -1.0f, -1.0f, 0.0f };
				}
				break;
			case 4://NativeScriptComponent
				if (!entity.HasComponent<NativeScriptComponent>()) entity.AddComponent<NativeScriptComponent>(std::string(), ScriptState::MustBeInitialized);
				break;
			}
		}
	}

	void DrawColorPicker(const char* label, glm::vec4& value, const UISettings& settings)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

		const float width = ImGui::GetContentRegionAvailWidth() - 5.0f * buttonSize.x + 18.0f;

		ImGui::PushMultiItemsWidths(4, width);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		int32 Color[4] = { static_cast<int32>(value.r * 255.0f), static_cast<int32>(value.g * 255.0f), static_cast<int32>(value.b * 255.0f), static_cast<int32>(value.a * 255.0f) };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("R", buttonSize))
		{
			value.r = settings.ResetValue;
			Color[0] = static_cast<int32>(value.r * 255.0f);
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##R", &Color[0], 1.0f, 0, 255))
			value.r = Color[0] / 255.0f;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("G", buttonSize))
		{
			value.g = settings.ResetValue;
			Color[1] = static_cast<int32>(value.g * 255.0f);
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if(ImGui::DragInt("##G", &Color[1], 1.0f, 0, 255))
			value.g = Color[1] / 255.0f;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("B", buttonSize))
		{
			value.b = settings.ResetValue;
			Color[2] = static_cast<int32>(value.b * 255.0f);
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##B", &Color[2], 1.0f, 0, 255))
			value.b = Color[2] / 255.0f;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("A", { buttonSize.y, buttonSize.y }))
		{
			value.a = settings.ResetValue;
			Color[3] = static_cast<int32>(value.a * 255.0f);
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##A", &Color[3], 1.0f, 0, 255))
			value.a = Color[3] / 255.0f;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::ColorEdit4("##", &value[0], ImGuiColorEditFlags_NoInputs);

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	bool DrawVec3Control(const char* label, glm::vec3& value, const UISettings& settings)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		bool xFlag, yFlag, zFlag;
		xFlag = yFlag = zFlag = false;

		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

		const float width = ImGui::GetContentRegionAvailWidth() - 3.0f * buttonSize.x + 8.0f;

		ImGui::PushMultiItemsWidths(3, width);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		
		if (settings.Disabled[0])
		{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			value.x = settings.ResetValue;
			xFlag = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &value.x, settings.Speed, settings.Clamp.x, settings.Clamp.y))
			xFlag = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (settings.Disabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}


		if (settings.Disabled[1])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			value.y = settings.ResetValue;
			yFlag = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &value.y, settings.Speed, settings.Clamp.x, settings.Clamp.y))
			yFlag = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (settings.Disabled[1])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (settings.Disabled[2])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			value.z = settings.ResetValue;
			zFlag = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &value.z, settings.Speed, settings.Clamp.x, settings.Clamp.y))
			zFlag = true;
		ImGui::PopItemWidth();
		if (settings.Disabled[2])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
		return (xFlag || yFlag || zFlag);
	}

	bool DrawVec2Control(const char* label, glm::vec2& value, const UISettings& settings)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		bool xFlag, yFlag;
		xFlag = yFlag = false;

		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

		const float width = ImGui::GetContentRegionAvailWidth() - 2.0f * buttonSize.x + 4.0f;

		ImGui::PushMultiItemsWidths(2, width);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		if (settings.Disabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			value.x = settings.ResetValue;
			xFlag = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &value.x, settings.Speed, settings.Clamp.x, settings.Clamp.y))
			xFlag = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (settings.Disabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (settings.Disabled[1])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			value.y = settings.ResetValue;
			yFlag = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &value.y, settings.Speed, settings.Clamp.x, settings.Clamp.y))
			yFlag = true;
		ImGui::PopItemWidth();
		if (settings.Disabled[1])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		
		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
		return (xFlag || yFlag);
	}

	bool DrawFloatControl(const char* label, float& value, const UISettings& settings)
	{
		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		if (settings.Disabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		bool changed;
		if(value<0.001f)
			changed = ImGui::DragFloat("##", &value, settings.Speed, settings.Clamp.x, settings.Clamp.y, "%.5f");
		else
			changed = ImGui::DragFloat("##", &value, settings.Speed, settings.Clamp.x, settings.Clamp.y);

		if (settings.Disabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::Columns(1);
		ImGui::PopID();
		return changed;
	}

	bool DrawIntControl(const char* label, int& value, const UISettings& settings)
	{
		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		if (settings.Disabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::DragInt("##", &value, settings.Speed, static_cast<int32>(settings.Clamp.x), static_cast<int32>(settings.Clamp.y));
		if (settings.Disabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::Columns(1);
		ImGui::PopID();
		return changed;
	}

	void DrawComboControl(const char* label, int32& index, const char* const* selection, int32 size, const UISettings& settings)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

		ImGui::Combo("##", &index, selection, size);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();
	}

	void DrawTextInput(const char* label, std::string& text, size_t length, const UISettings& settings)
	{
		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()+3.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		char* buffer = new char[length];
		memcpy(buffer, text.c_str(), text.size()+1);

		if (ImGui::InputText("##", buffer, length))
			text = std::string(buffer);
		delete[] buffer;

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		
		ImGui::Columns(1);
		ImGui::PopID();
	}


	bool DrawMenuItem(const char* icon, const char* item, const char* shortcut)
	{
		float offset = 200.0f;
		ImGuiIO& io = ImGui::GetIO();
		ImGuiContext& g = *ImGui::GetCurrentContext();
		auto iconsFont = io.Fonts->Fonts[0];
		ImGui::PushFont(iconsFont);
		offset -= ImGui::CalcTextSize(icon).x;
		bool pressed = false;
		ImGui::Selectable(icon, &pressed, ImGuiSelectableFlags_SelectOnRelease | ImGuiSelectableFlags_SetNavIdOnHover);
		ImGui::SameLine();
		ImGui::PopFont();

		offset -= ImGui::CalcTextSize(item).x + ImGui::CalcTextSize(shortcut).x;
		ImGui::Text(item);
		ImGui::SameLine(0.0f, offset);
		ImGui::PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
		ImGui::Text(shortcut);
		ImGui::PopStyleColor();

		return pressed;
	}

	bool DrawFilePicker(const char* label, std::string& text, const char* extension, const UISettings& settings)
	{
		constexpr char PNG[] = "PNG file (*.png)\0*.png\0";
		constexpr char OBJ[] = "3D Object (*.obj)\0*.obj\0";

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.0f, 1.0f});
		
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight, lineHeight);

		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - buttonSize.x);

		std::string temp = utils::strip_path(text);
		char* buffer = new char[temp.size()+1];
		memcpy(buffer, temp.c_str(), temp.size() + 1);
		ImGui::InputText("##", buffer, temp.size());
		delete[] buffer;

		ImGui::PopItemWidth();
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();

		ImGui::SameLine();

		bool changed = false;
		if (ImGui::Button("...", buttonSize))
		{
			std::string filepath;
			if (std::string(extension).compare(".obj") == 0)
				filepath = CreateFileDialog(FileDialogType::Open, OBJ);
			else
				filepath = CreateFileDialog(FileDialogType::Open, PNG);
			if (!filepath.empty() && text.compare(filepath) != 0)
			{
				text = filepath;
				changed = true;
			}
		}

		ImGui::PopStyleVar();
		
		ImGui::Columns(1);
		ImGui::PopID();
		return changed;
	}

	//TODO: Probably Change Clamp Behaviour
	bool DrawIVec2Control(const char* label, glm::ivec2& value, const UISettings& settings)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		bool xFlag, yFlag;
		xFlag = yFlag = false;

		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

		const float width = ImGui::GetContentRegionAvailWidth() - 2.0f * buttonSize.x + 4.0f;

		ImGui::PushMultiItemsWidths(2, width);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		if (settings.Disabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		ImGui::Button("X", buttonSize);
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##X", &value.x, settings.Speed, 0, static_cast<int32>(settings.Clamp.x)))
			xFlag = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (settings.Disabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (settings.Disabled[1])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushFont(boldFont);
		ImGui::Button("Y", buttonSize);
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##Y", &value.y, settings.Speed, 0, static_cast<int32>(settings.Clamp.y)))
			yFlag = true;
		ImGui::PopItemWidth();
		if (settings.Disabled[1])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
		return (xFlag || yFlag);
	}

	//TODO: Check return statement
	bool DrawTextureCoordinates(TextureCoordinates& textCoords, uint32 width, uint32 height, const UISettings& settings)
	{
		ImGui::PushID("Texture Coordinates");
		UISettings coordsSettings;
		coordsSettings.Clamp = glm::vec2(width, height);
		//Bottom Left
		glm::ivec2 coords = glm::ivec2(textCoords.BottomLeft.x * width, textCoords.BottomLeft.y * height);
		if (DrawIVec2Control("Bottom Left", coords, coordsSettings))
			textCoords.BottomLeft = glm::vec2((float)coords.x / (float)width, (float)coords.y / (float)height);
		//Bottom Right
		coords = glm::ivec2(textCoords.BottomRight.x * width, textCoords.BottomRight.y * height);
		if (DrawIVec2Control("Bottom Right", coords, coordsSettings))
			textCoords.BottomRight = glm::vec2((float)coords.x / (float)width, (float)coords.y / (float)height);
		//Top Right
		coords = glm::ivec2(textCoords.TopRight.x * width, textCoords.TopRight.y * height);
		if (DrawIVec2Control("Top Right", coords, coordsSettings))
			textCoords.TopRight = glm::vec2((float)coords.x / (float)width, (float)coords.y / (float)height);
		//Top Left
		coords = glm::ivec2(textCoords.TopLeft.x * width, textCoords.TopLeft.y * height);
		if (DrawIVec2Control("Top Left", coords, coordsSettings))
			textCoords.TopLeft = glm::vec2((float)coords.x / (float)width, (float)coords.y / (float)height);
		ImGui::PopID();
		return false;
	}

	bool DrawCheckboxControl(const char* label, bool& value, const UISettings& settings)
	{
		ImGui::PushID(label);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, settings.ColumnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		ImGui::Text("");
		ImGui::SameLine(0.0f, ImGui::GetContentRegionAvailWidth() - ImGui::GetFrameHeight());
		const bool changed = ImGui::Checkbox("##", &value);
		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();

		return changed;
	}

	bool DrawMaterialControl(const char* label, Material& mat, const UISettings& settings)
	{
		static Ref<Asset> TransparentTexture;
		TransparentTexture = AssetManager::RequestTexture("../Assets/Textures/Editor/Transparency.png");
		ImGui::PushID(label);
		if (ImGui::CollapsingHeader("Albedo", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Diffuse");
			GPU::Texture2D* texture;
			void* img = nullptr;
			if (mat.DiffuseTexture->Type == AssetType::INVALID || mat.DiffuseTexture->Type == AssetType::LOADING)
			{
				texture = static_cast<GPU::Texture2D*>(TransparentTexture->ActualAsset);
				img = texture->GetID();
			}
			else
			{
				texture = static_cast<GPU::Texture2D*>(mat.DiffuseTexture->ActualAsset);
				img = texture->GetID();
			}
			ImGui::Image(img, { 64.0f, 64.0f });
			if (ImGui::IsItemClicked())
			{
				std::string filepath = CreateFileDialog(FileDialogType::Open, "PNG file (*.png); JPG file (*.jpg)\0*.png;*.jpg\0");
				if (!filepath.empty())
				{
					mat.DiffuseName = filepath;
					mat.DiffuseTexture = AssetManager::RequestTexture(filepath.c_str());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Tint Color");
			ImGui::SameLine();
			UISettings subsettings;
			subsettings.ColumnWidth = 64.0f + ImGui::CalcTextSize("Tint Color").x + 20.0f;
			DrawColorPicker("", mat.Diffuse, subsettings);
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Normals", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Bump");
			GPU::Texture2D* texture;
			void* img = nullptr;
			if (mat.BumpTexture->Type == AssetType::INVALID || mat.BumpTexture->Type == AssetType::LOADING)
			{
				texture = static_cast<GPU::Texture2D*>(TransparentTexture->ActualAsset);
				img = texture->GetID();
			}
			else
			{
				texture = static_cast<GPU::Texture2D*>(mat.BumpTexture->ActualAsset);
				img = texture->GetID();
			}
			ImGui::Image(img, { 64.0f, 64.0f });
			if (ImGui::IsItemClicked())
			{
				std::string filepath = CreateFileDialog(FileDialogType::Open, "PNG file (*.png); JPG file (*.jpg)\0*.png;*.jpg\0");
				if (!filepath.empty())
				{
					mat.BumpName = filepath;
					mat.BumpTexture = AssetManager::RequestTexture(filepath.c_str());
				}
			}
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Ambient", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Ambient");
			GPU::Texture2D* texture;
			void* img = nullptr;
			if (mat.AmbientTexture->Type == AssetType::INVALID || mat.AmbientTexture->Type == AssetType::LOADING)
			{
				texture = static_cast<GPU::Texture2D*>(TransparentTexture->ActualAsset);
				img = texture->GetID();
			}
			else
			{
				texture = static_cast<GPU::Texture2D*>(mat.AmbientTexture->ActualAsset);
				img = texture->GetID();
			}
			ImGui::Image(img, { 64.0f, 64.0f });
			if (ImGui::IsItemClicked())
			{
				std::string filepath = CreateFileDialog(FileDialogType::Open, "PNG file (*.png); JPG file (*.jpg)\0*.png;*.jpg\0");
				if (!filepath.empty())
				{
					mat.AmbientName = filepath;
					mat.AmbientTexture = AssetManager::RequestTexture(filepath.c_str());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Tint Color");
			ImGui::SameLine();
			UISettings subsettings;
			subsettings.ColumnWidth = 64.0f + ImGui::CalcTextSize("Tint Color").x + 20.0f;
			DrawColorPicker("", mat.Ambient, subsettings);
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Specular", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Specular");
			GPU::Texture2D* texture;
			void* img = nullptr;
			if (mat.SpecularTexture->Type == AssetType::INVALID || mat.SpecularTexture->Type == AssetType::LOADING)
			{
				texture = static_cast<GPU::Texture2D*>(TransparentTexture->ActualAsset);
				img = texture->GetID();
			}
			else
			{
				texture = static_cast<GPU::Texture2D*>(mat.SpecularTexture->ActualAsset);
				img = texture->GetID();
			}
			ImGui::Image(img, { 64.0f, 64.0f });
			if (ImGui::IsItemClicked())
			{
				std::string filepath = CreateFileDialog(FileDialogType::Open, "PNG file (*.png); JPG file (*.jpg)\0*.png;*.jpg\0");
				if (!filepath.empty())
				{
					mat.SpecularName = filepath;
					mat.SpecularTexture = AssetManager::RequestTexture(filepath.c_str());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Tint Color");
			ImGui::SameLine();
			UISettings subsettings;
			subsettings.ColumnWidth = 64.0f + ImGui::CalcTextSize("Tint Color").x + 20.0f;
			DrawColorPicker("", mat.Specular, subsettings);
			ImGui::PopID();
		}
		ImGui::PopID();

		return false;
	}

}