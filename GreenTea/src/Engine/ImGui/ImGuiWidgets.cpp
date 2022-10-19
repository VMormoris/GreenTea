#include "ImGuiWidgets.h"
#include <Engine/Core/Context.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <IconsForkAwesome.h>

static void DrawPrefix(const char* label, float width, const std::string& help);
static void DrawPostfix(void);
static bool DrawCoordinate(const char* label, glm::vec2& value, const glm::vec2& resets, uint32 width, uint32 height, float colWidth, const std::string& help);

namespace gte::gui {

	bool DrawSearchbar(const char* label, std::string& text, size_t length, float width)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto IconFont = io.Fonts->Fonts[0];
		auto BoldFont = io.Fonts->Fonts[1];
		const float lineHeight = BoldFont->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;

		ImGui::PushID(label);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
		bool shouldFocus = false;
		if (text.empty())//Check if hint should be showned so we know if we need to dislpay icon as well
		{
			ImVec2 size(lineHeight + 3.0f, lineHeight);
			ImGui::PushFont(IconFont);
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.063f, 0.063f, 0.063f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.063f, 0.063f, 0.063f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.063f, 0.063f, 0.063f, 1.0f });
			shouldFocus = ImGui::Button(ICON_FK_SEARCH, size);
			ImGui::PopStyleColor(3);
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2.0f);
		}

		//Actual Input
		ImGui::PushItemWidth(width);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.063f, 0.063f, 0.063f, 1.0f });

		char* buffer = new char[length + 1];
		memcpy(buffer, text.c_str(), text.size() + 1);
		if (shouldFocus) ImGui::SetKeyboardFocusHere();
		bool changed = ImGui::InputTextWithHint("##Searchbar", "Search...", buffer, length);
		if (changed)
		{
			text = std::string(buffer);
			std::for_each(text.begin(), text.end(), [](char& c) { c = std::tolower(c); });
		}
		delete[] buffer;

		ImGui::PopStyleColor();
		ImGui::PopItemWidth();

		if (!text.empty() && !changed)//Button for clearing text
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2.0f);

			ImVec2 size(lineHeight + 3.0f, lineHeight);
			ImGui::PushFont(IconFont);
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.063f, 0.063f, 0.063f, 1.0f });
			const bool shouldClear = ImGui::Button(ICON_FK_TIMES, size);
			ImGui::PopStyleColor();
			ImGui::PopFont();

			if (shouldClear)
			{
				text = "";
				changed = true;
			}
		}
		ImGui::PopStyleVar(2);
		ImGui::PopID();
		return changed;
	}

	bool DrawBreadcrumb(const char* label, std::filesystem::path& path)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto IconFont = io.Fonts->Fonts[0];
		auto BoldFont = io.Fonts->Fonts[1];

		std::vector<std::filesystem::path> breadcrumb;
		auto cp = path;
		while (cp.has_parent_path())
		{
			const auto dir = cp;
			//if(!dir.empty())
			breadcrumb.push_back(dir);
			cp = cp.parent_path();
		}
		breadcrumb.push_back("Assets");

		bool changed = false;
		ImGui::PushID(label);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.105f, 0.11f, 1.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
		for (auto it = breadcrumb.crbegin(); it != breadcrumb.crend(); ++it)
		{
			if (it != breadcrumb.crbegin())
				ImGui::SameLine();

			const auto dir = *it;
			auto display = std::filesystem::relative(dir, "Assets").filename().string();
			if (display.compare(".") == 0)
				display = "Assets";
			if (ImGui::Button(display.c_str()))
			{
				changed = true;
				path = dir;
			}

			if (it == breadcrumb.crend() - 1)
				continue;
			ImGui::SameLine();
			ImGui::PushFont(BoldFont);
			ImGui::Text("/");
			ImGui::PopFont();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopID();

		return changed;
	}

	bool DrawEditText(std::string& text, size_t length, bool& editing)
	{
		static bool shouldFocus = false;
		bool edited = false;
		if (!editing)
		{
			ImGui::Text(text.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				editing = true;
				shouldFocus = true;
			}
		}
		else
		{
			char tbuffer[64];
			memcpy(tbuffer, text.c_str(), text.size() + 1);
			if (shouldFocus)
			{
				ImGui::SetKeyboardFocusHere();
				shouldFocus = false;
			}
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.063f, 0.063f, 0.063f, 1.0f });
			if (ImGui::InputText("##", tbuffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				text = std::string(tbuffer);
				edited = true;
				editing = false;
			}
			ImGui::PopStyleColor();
			if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemHovered())
				editing = false;
		}
		return edited;
	}

	bool DrawMenuItem(const char* icon, const char* item, const char* shortcut, const char* biggest)
	{
		constexpr float IconOffset = 34.0f;
		const float Width = ImGui::CalcTextSize(biggest).x + IconOffset + 4.0f;
		const float ShortcutOffset = Width - ImGui::CalcTextSize(shortcut).x;

		ImGuiIO& io = ImGui::GetIO();

		auto IconsFont = io.Fonts->Fonts[3];
		ImGui::PushFont(IconsFont);
		bool pressed = false;
		ImGui::Selectable(icon, &pressed, ImGuiSelectableFlags_SelectOnRelease | ImGuiSelectableFlags_SetNavIdOnHover);
		ImGui::PopFont();

		ImGui::SameLine(IconOffset);
		ImGui::Text(item);

		if (shortcut)
		{
			ImGui::SameLine(ShortcutOffset);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_TextDisabled]);
			ImGui::Text(shortcut);
			ImGui::PopStyleColor();
		}

		return pressed;
	}

	bool DrawBoolControl(const char* label, bool& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::Dummy({0.0f, 0.0f});
		ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 19.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::Checkbox("##", &value);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		DrawPostfix();
		return changed;
	}

	bool DrawCharControl(const char* label, char& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		//Limit Downcast
		char min = (char)settings.MinInt;
		char max = (char)settings.MaxInt;
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_S8, &value, settings.Speed, &min, &max);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawInt16Control(const char* label, int16& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		//Limit Downcast
		int16 min = (int16)settings.MinInt;
		int16 max = (int16)settings.MaxInt;
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_S16, &value, settings.Speed, &min, &max);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawInt32Control(const char* label, int32& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		//Limit Downcast
		int32 min = (int32)settings.MinInt;
		int32 max = (int32)settings.MaxInt;
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_S32, &value, settings.Speed, &min, &max);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawInt64Control(const char* label, int64& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_S64, &value, settings.Speed, &settings.MinInt, &settings.MaxInt);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawByteControl(const char* label, byte& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		//Limit Downcast
		byte min = (byte)settings.MinInt;
		byte max = (byte)settings.MaxInt;
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_U8, &value, settings.Speed, &min, &max);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawUint16Control(const char* label, uint16& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		//Limit Downcast
		uint16 min = (uint16)settings.MinInt;
		uint16 max = (uint16)settings.MaxInt;
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_U16, &value, settings.Speed, &min, &max);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawUint32Control(const char* label, uint32& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		//Limit Downcast
		uint32 min = (uint32)settings.MinInt;
		uint32 max = (uint32)settings.MaxInt;
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_U32, &value, settings.Speed, &min, &max);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawUint64Control(const char* label, uint64& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_U64, &value, settings.Speed, &settings.MinUint, &settings.MaxUint);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawFloatControl(const char* label, float& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::DragFloat("##", &value, settings.Speed, static_cast<float>(settings.MinFloat), static_cast<float>(settings.MaxFloat));
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawDoubleControl(const char* label, double& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::DragScalar("##", ImGuiDataType_Double, &value, settings.Speed, &settings.MinFloat, &settings.MaxFloat);
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawStringControl(const char* label, std::string& value, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		char* toedit = new char[settings.Length + 1];
		memcpy(toedit, value.c_str(), value.size() + 1);
		const bool changed = ImGui::InputText("##", toedit, settings.Length);
		if (changed)
			value = std::string(toedit);
		delete[] toedit;
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		return changed;
	}

	bool DrawVec2Control(const char* label, glm::vec2& value, const UISettings& settings, const std::string& help)
	{
		bool changed = false;
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		DrawPrefix(label, settings.ColumnWidth, help);

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth() + 31.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 1 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			value.x = settings.ResetValue;
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::DragFloat("##X", &value.x, settings.Speed, static_cast<float>(settings.MinFloat), static_cast<float>(settings.MaxFloat), "%.2f"))
			changed = true;
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			value.y = settings.ResetValue;
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (!settings.Enabled[1])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::DragFloat("##Y", &value.y, settings.Speed, static_cast<float>(settings.MinFloat), static_cast<float>(settings.MaxFloat), "%.2f"))
			changed = true;
		if (!settings.Enabled[1])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		DrawPostfix();
		return changed;
	}

	bool DrawVec3Control(const char* label, glm::vec3& value, const UISettings& settings, const std::string& help)
	{
		bool changed = false;
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		DrawPrefix(label, settings.ColumnWidth, help);

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() + 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 1 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			value.x = settings.ResetValue;
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::DragFloat("##X", &value.x, settings.Speed, static_cast<float>(settings.MinFloat), static_cast<float>(settings.MaxFloat), "%.2f"))
			changed = true;
		if (!settings.Enabled[0])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			value.y = settings.ResetValue;
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (!settings.Enabled[1])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::DragFloat("##Y", &value.y, settings.Speed, static_cast<float>(settings.MinFloat), static_cast<float>(settings.MaxFloat), "%.2f"))
			changed = true;
		if (!settings.Enabled[1])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			value.z = settings.ResetValue;
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (!settings.Enabled[2])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::DragFloat("##Z", &value.z, settings.Speed, static_cast<float>(settings.MinFloat), static_cast<float>(settings.MaxFloat), "%.2f"))
			changed = true;
		if (!settings.Enabled[2])
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		DrawPostfix();
		return changed;
	}

	bool DrawColorPicker(const char* label, glm::vec4& value, const UISettings& settings, const std::string& help)
	{
		bool changed = false;
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];

		DrawPrefix(label, settings.ColumnWidth, help);

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight + 3.0f, lineHeight);

		const float width = ImGui::GetContentRegionAvail().x - 5.0f * buttonSize.x + 23.0f;

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
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##R", &Color[0], 1.0f, 0, 255))
		{
			value.r = Color[0] / 255.0f;
			changed = true;
		}
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
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##G", &Color[1], 1.0f, 0, 255))
		{
			value.g = Color[1] / 255.0f;
			changed = true;
		}
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
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##B", &Color[2], 1.0f, 0, 255))
		{
			value.b = Color[2] / 255.0f;
			changed = true;
		}
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
			changed = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::DragInt("##A", &Color[3], 1.0f, 0, 255))
		{
			value.a = Color[3] / 255.0f;
			changed = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::ColorEdit4("##", &value[0], ImGuiColorEditFlags_NoInputs);

		ImGui::PopStyleVar();
		DrawPostfix();
		return changed;
	}

	bool DrawComboControl(const char* label, int32& index, const char* const* options, int32 size, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		const bool changed = ImGui::Combo("##", &index, options, size);
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		DrawPostfix();
		return changed;
	}

	bool DrawEnumControl(const char* label, const std::string& type_name, internal::Enum::Value& value, internal::FieldType type, const UISettings& settings, const std::string& help)
	{
		using namespace internal;
		if (!GetContext()->ScriptEngine->HasEnum(type_name))
			return false;

		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		
		const Enum& enumaration = GetContext()->ScriptEngine->GetEnum(type_name);
		int32 index = -1;
		const char** names = new const char*[enumaration.Values.size()];
		for (int32 i = 0; i < enumaration.Values.size(); i++)
		{
			const Enum::Value& val = enumaration.Values[i];
			names[i] = val.Name.c_str();
			switch (type)
			{
			case FieldType::Enum_Char:
				if (val.Char == value.Char)
					index = i;
				break;
			case FieldType::Enum_Int16:
				if (val.Int16 == value.Int16)
					index = i;
				break;
			case FieldType::Enum_Int32:
				if (val.Int32 == value.Int32)
					index = i;
				break;
			case FieldType::Enum_Int64:
				if (val.Int64 == value.Int64)
					index = i;
				break;
			case FieldType::Enum_Byte:
				if (val.Byte == value.Byte)
					index = i;
				break;
			case FieldType::Enum_Uint16:
				if (val.Uint16 == value.Uint16)
					index = i;
				break;
			case FieldType::Enum_Uint32:
				if (val.Uint32 == value.Uint32)
					index = i;
				break;
			case FieldType::Enum_Uint64:
				if (val.Uint64 == value.Uint64)
					index = i;
				break;
			}
		}
		if (!settings.Enabled[0])
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		const bool changed = ImGui::Combo("##", &index, names, static_cast<int32>(enumaration.Values.size()));
		if (!settings.Enabled[0])
		{
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		}

		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		DrawPostfix();
		delete[] names;
		if (changed)
			value = enumaration.Values[index];
		return changed;
	}

	bool DrawVec2BoolControl(const char* label, bool& x, bool& y, const UISettings& settings, const std::string& help)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[1];
		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize(lineHeight, lineHeight);

		DrawPrefix(label, settings.ColumnWidth, help);
		ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x + 20.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(boldFont);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button("X", buttonSize);
		ImGui::PopItemFlag();
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		bool changed = ImGui::Checkbox("##x", &x);
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushFont(boldFont);
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Button("Y", buttonSize);
		ImGui::PopItemFlag();
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		if (ImGui::Checkbox("##y", &y))
			changed = true;

		ImGui::PopStyleVar();
		DrawPostfix();
		return changed;
	}

	bool DrawTextureCoordinates(TextureCoordinates& coords, uint32 width, uint32 height, const UISettings& settings)
	{
		const float colWidth = settings.ColumnWidth - 20.0f;
		bool changed = DrawCoordinate("Bottom Left", coords.BottomLeft, { 0.0f, 0.0f }, width, height, colWidth, "Bottom Left");
		if (DrawCoordinate("Bottom Right", coords.BottomRight, { 1.0f, 0.0f }, width, height, colWidth, "Bottom Right"))
			changed = true;
		if (DrawCoordinate("Top Right", coords.TopRight, { 1.0f, 1.0f }, width, height, colWidth, "Top Right"))
			changed = true;
		if (DrawCoordinate("Top Left", coords.TopLeft, { 0.0f, 1.0f }, width, height, colWidth, "Top Left"))
			changed = true;
		return changed;
	}

	bool DrawAssetControl(const char* label, uuid& id, const char* filetype, const UISettings& settings, const std::string& help)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto IconsFont = io.Fonts->Fonts[3];

		DrawPrefix(label, settings.ColumnWidth, help);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.063f, 0.063f, 0.063f, 1.0f });

		std::filesystem::path name = internal::GetContext()->AssetWatcher.GetFilepath(id);
		const float offset = name.empty() ? 18.5f : 37.0f;
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - offset);
		ImGui::InputText("##", (char*)name.stem().string().c_str(), name.stem().string().size());//Safe to cast to char* as is basically disabled
		ImGui::PopItemWidth();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopItemFlag();
		ImGui::SameLine(0.0f, 4.0f);

		bool changed = false;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				std::filesystem::path filepath = (const char*)payload->Data;
				if (filepath.extension() == filetype || std::string(filetype).compare("*") == 0)
				{
					changed = true;
					id = internal::GetContext()->AssetWatcher.GetID(filepath.string());
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4.0f);
		ImGui::PushFont(IconsFont);
		if (!name.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.063f, 0.063f, 0.063f, 0.5f });
			if (ImGui::Button(ICON_FK_TIMES))
			{
				id = {};
				changed = true;
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
		}
		if (ImGui::Button(ICON_FK_DOT_CIRCLE_O))
			ImGui::OpenPopup("Asset Selection");
		ImGui::PopFont();
		if (ImGui::BeginPopup("Asset Selection"))
		{
			const std::vector<uuid>& ids = internal::GetContext()->AssetWatcher.GetAssets({ filetype });
			for (const auto& tid : ids)
			{
				std::filesystem::path filepath = internal::GetContext()->AssetWatcher.GetFilepath(tid);
				if (ImGui::MenuItem(filepath.stem().string().c_str()))
				{
					id = tid;
					changed = true;
				}
			}
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar();

		DrawPostfix();
		return changed;
	}

	bool DrawEntityControl(const char* label, uuid& id, const UISettings& settings, const std::string& help)
	{
		DrawPrefix(label, settings.ColumnWidth, help);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.063f, 0.063f, 0.063f, 1.0f });
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 5.0f);
		std::string tag = internal::GetContext()->ActiveScene->FindEntityWithUUID(id).GetName();
		ImGui::InputText("##", (char*)tag.c_str(), tag.size());//Casting is safe since value cannot be changed
		bool changed = false;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
			{
				uuid temp_id = *(uuid*)payload->Data;
				if (temp_id.IsValid())
				{
					changed = true;
					id = temp_id;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopItemFlag();

		ImGui::PopStyleVar();

		DrawPostfix();
		return changed;
	}

	bool DrawFieldControl(const internal::FieldSpecification& spec, void* buffer)
	{
		const float colWidth = ImGui::CalcTextSize(spec.Name.c_str()).x + 8.0f;
		UISettings settings;
		settings.ColumnWidth = colWidth > 136 ? colWidth : 136;

		void* ptr = (byte*)buffer + (internal::GetContext()->Playing ? spec.Offset : spec.BufferOffset);
		bool changed = false;
		switch (spec.Type)
		{
		using namespace internal;
		case FieldType::Bool:
			changed = DrawBoolControl(spec.Name.c_str(), *(bool*)ptr, settings);
			break;
		case FieldType::Char:
			settings.MinInt = spec.MinInt;
			settings.MaxInt = spec.MaxInt;
			settings.Speed = 1.0f;
			changed = DrawCharControl(spec.Name.c_str(), *(char*)ptr, settings);
			break;
		case FieldType::Int16:
			settings.MinInt = spec.MinInt;
			settings.MaxInt = spec.MaxInt;
			settings.Speed = 1.0f;
			changed = DrawInt16Control(spec.Name.c_str(), *(int16*)ptr, settings);
			break;
		case FieldType::Int32:
			settings.MinInt = spec.MinInt;
			settings.MaxInt = spec.MaxInt;
			settings.Speed = 1.0f;
			changed = DrawInt32Control(spec.Name.c_str(), *(int32*)ptr, settings);
			break;
		case FieldType::Int64:
			settings.MinInt = spec.MinInt;
			settings.MaxInt = spec.MaxInt;
			settings.Speed = 1.0f;
			changed = DrawInt64Control(spec.Name.c_str(), *(int64*)ptr, settings);
			break;
		case FieldType::Byte:
			settings.MinUint = spec.MinInt;
			settings.MaxUint = spec.MaxUint;
			settings.Speed = 1.0f;
			changed = DrawByteControl(spec.Name.c_str(), *(byte*)ptr, settings);
			break;
		case FieldType::Uint16:
			settings.MinUint = spec.MinInt;
			settings.MaxUint = spec.MaxUint;
			settings.Speed = 1.0f;
			changed = DrawUint16Control(spec.Name.c_str(), *(uint16*)ptr, settings);
			break;
		case FieldType::Uint32:
			settings.MinUint = spec.MinInt;
			settings.MaxUint = spec.MaxUint;
			settings.Speed = 1.0f;
			changed = DrawUint32Control(spec.Name.c_str(), *(uint32*)ptr, settings);
			break;
		case FieldType::Uint64:
			settings.MinUint = spec.MinInt;
			settings.MaxUint = spec.MaxUint;
			settings.Speed = 1.0f;
			changed = DrawUint64Control(spec.Name.c_str(), *(uint64*)ptr, settings);
			break;
		case FieldType::Float32:
			settings.MinFloat = spec.MinFloat;
			settings.MaxFloat = spec.MaxFloat;
			changed = DrawFloatControl(spec.Name.c_str(), *(float*)ptr, settings);
			break;
		case FieldType::Float64:
			settings.MinFloat = spec.MinFloat;
			settings.MaxFloat = spec.MaxFloat;
			changed = DrawDoubleControl(spec.Name.c_str(), *(double*)ptr, settings);
			break;
		case FieldType::String:
			settings.Length = spec.Length + 1;
			changed = DrawStringControl(spec.Name.c_str(), *(std::string*)ptr, settings);
			break;
		case FieldType::Vec2:
			settings.MinFloat = spec.MinFloat;
			settings.MaxFloat = spec.MaxFloat;
			changed = DrawVec2Control(spec.Name.c_str(), *(glm::vec2*)ptr, settings);
			break;
		case FieldType::Vec3:
			settings.MinFloat = spec.MinFloat;
			settings.MaxFloat = spec.MaxFloat;
			changed = DrawVec3Control(spec.Name.c_str(), *(glm::vec3*)ptr, settings);
			break;
		case FieldType::Vec4:
			settings.MinFloat = spec.MinFloat;
			settings.MaxFloat = spec.MaxFloat;
			changed = DrawColorPicker(spec.Name.c_str(), *(glm::vec4*)ptr, settings);
			break;
		case FieldType::Asset:
		{
			Ref<Asset>& ref = *(Ref<Asset>*)ptr;
			changed = DrawAssetControl(spec.Name.c_str(), ref->ID, "*", settings);
			break;
		}
		case FieldType::Entity:
		{
			Entity& entity = *(Entity*)ptr;
			uuid id = entity.GetID();
			changed = DrawEntityControl(spec.Name.c_str(), id, settings);
			if (changed)
				entity = internal::GetContext()->ActiveScene->FindEntityWithUUID(id);
			break;
		}
		case FieldType::Enum_Char:
		{
			Enum::Value val;
			val.Char = *(char*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(char*)ptr = val.Char;
			break;
		}
		case FieldType::Enum_Int16:
		{
			Enum::Value val;
			val.Int16 = *(int16*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(int16*)ptr = val.Int16;
			break;
		}
		case FieldType::Enum_Int32:
		{
			Enum::Value val;
			val.Int32 = *(int32*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(int32*)ptr = val.Int32;
			break;
		}
		case FieldType::Enum_Int64:
		{
			Enum::Value val;
			val.Int64 = static_cast<char>(*(int64*)ptr);
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(int64*)ptr = val.Int64;
			break;
		}
		case FieldType::Enum_Byte:
		{
			Enum::Value val;
			val.Byte = *(byte*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(byte*)ptr = val.Byte;
			break;
		}
		case FieldType::Enum_Uint16:
		{
			Enum::Value val;
			val.Uint16 = *(uint16*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(uint16*)ptr = val.Uint16;
			break;
		}
		case FieldType::Enum_Uint32:
		{
			Enum::Value val;
			val.Uint32 = *(uint32*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(uint32*)ptr = val.Uint32;
			break;
		}
		case FieldType::Enum_Uint64:
		{
			Enum::Value val;
			val.Uint64 = *(uint64*)ptr;
			changed = DrawEnumControl(spec.Name.c_str(), spec.TypeName, val, spec.Type, settings);
			if (changed)
				*(uint64*)ptr = val.Uint64;
			break;
		}
		}
		return changed;
	}
}

void DrawPrefix(const char* label, float width, const std::string& help)
{
	ImGui::PushID(label);
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, width);
	ImGui::Text(label);
	if (ImGui::IsItemHovered() && !help.empty())
	{
		ImGui::BeginTooltip();
		ImGui::Text(help.c_str());
		ImGui::EndTooltip();
	}
	ImGui::NextColumn();
}

void DrawPostfix(void)
{
	ImGui::Columns(1);
	ImGui::PopID();
}

bool DrawCoordinate(const char* label, glm::vec2& value, const glm::vec2& resets, uint32 width, uint32 height, float colWidth, const std::string& help)
{
	constexpr uint32 min = 0;
	bool changed = false;
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[1];

	DrawPrefix(label, colWidth, help);
	ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth() + 31.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 1 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
	{
		value.x = resets.x;
		changed = true;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	uint32 x = static_cast<uint32>(value.x * width);
	if (ImGui::DragScalar("##X", ImGuiDataType_U32, &x, 1.0f, &min, &width))
	{
		value.x = static_cast<float>(x) / static_cast<float>(width);
		changed = true;
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
	{
		value.y = resets.y;
		changed = true;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	uint32 y = static_cast<uint32>(value.y * height);
	if (ImGui::DragScalar("##Y", ImGuiDataType_U32, &y, 1.0f, &min, &height))
	{
		value.y = static_cast<float>(y) / static_cast<float>(height);
		changed = true;
	}
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	DrawPostfix();
	return changed;
}