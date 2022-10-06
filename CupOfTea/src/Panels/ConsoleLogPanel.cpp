#include "ConsoleLogPanel.h"

#include <imgui.h>

void ConsoleLogPanel::Draw(void)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];
	auto buttonActive = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
	buttonActive.w = 0.8f;
	auto* logger = gte::internal::Logger::Get();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	if(ImGui::Button("Clear"))
		logger->Clear();
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushFont(IconsFont);
	auto filter = mFilter;//Store filter to local variable for changes because we used the normal for Pushing and poping colors
	if ((mFilter & gte::internal::Logger::Type::TRACE) == gte::internal::Logger::Type::TRACE)
		ImGui::PushStyleColor(ImGuiCol_Button, buttonActive);
	if (ImGui::Button(ICON_FK_SEARCH))
	{
		if ((filter & gte::internal::Logger::Type::TRACE) == gte::internal::Logger::Type::TRACE)
			filter &= ~gte::internal::Logger::Type::TRACE;
		else
			filter |= gte::internal::Logger::Type::TRACE;
	}
	if ((mFilter & gte::internal::Logger::Type::TRACE) == gte::internal::Logger::Type::TRACE)
		ImGui::PopStyleColor();
	ImGui::SameLine();
	if ((mFilter & gte::internal::Logger::Type::INFO) == gte::internal::Logger::Type::INFO)
		ImGui::PushStyleColor(ImGuiCol_Button, buttonActive);
	if (ImGui::Button(ICON_FK_INFO_CIRCLE))
	{
		if ((filter & gte::internal::Logger::Type::INFO) == gte::internal::Logger::Type::INFO)
			filter &= ~gte::internal::Logger::Type::INFO;
		else
			filter |= gte::internal::Logger::Type::INFO;
	}
	ImGui::SameLine();
	if ((mFilter & gte::internal::Logger::Type::INFO) == gte::internal::Logger::Type::INFO)
		ImGui::PopStyleColor();
	if ((mFilter & gte::internal::Logger::Type::WARNING) == gte::internal::Logger::Type::WARNING)
		ImGui::PushStyleColor(ImGuiCol_Button, buttonActive);
	ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.9f, 0.0f, 1.0f });
	if (ImGui::Button(ICON_FK_EXCLAMATION_TRIANGLE))
	{
		if ((filter & gte::internal::Logger::Type::WARNING) == gte::internal::Logger::Type::WARNING)
			filter &= ~gte::internal::Logger::Type::WARNING;
		else
			filter |= gte::internal::Logger::Type::WARNING;
	}
	ImGui::SameLine();
	ImGui::PopStyleColor();
	if ((mFilter & gte::internal::Logger::Type::WARNING) == gte::internal::Logger::Type::WARNING)
		ImGui::PopStyleColor();
	if ((mFilter & gte::internal::Logger::Type::ERR) == gte::internal::Logger::Type::ERR)
		ImGui::PushStyleColor(ImGuiCol_Button, buttonActive);
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.88f, 0.023f, 0.0f, 1.0f });
	if (ImGui::Button(ICON_FK_EXCLAMATION_CIRCLE))
	{
		if ((filter & gte::internal::Logger::Type::ERR) == gte::internal::Logger::Type::ERR)
			filter &= ~gte::internal::Logger::Type::ERR;
		else
			filter |= gte::internal::Logger::Type::ERR;
	}
	if ((mFilter & gte::internal::Logger::Type::ERR) == gte::internal::Logger::Type::ERR)
		ImGui::PopStyleColor();
	mFilter = filter;
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::Separator();

	ImGui::BeginChild("actual logs");
	auto& history = logger->GetHistory();
	for (const auto& [type, log] : history)
	{
		if (mFilter == gte::internal::Logger::Type::NONE || (mFilter & type) == type)
			DrawLine(type, log);
	}
	ImGui::EndChild();
}

void ConsoleLogPanel::DrawLine(gte::internal::Logger::Type type, const std::string& msg)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];
	ImGui::PushFont(IconsFont);
	switch (type)
	{
	case gte::internal::Logger::Type::TRACE:
		ImGui::Text(ICON_FK_SEARCH);
		break;
	case gte::internal::Logger::Type::INFO:
		ImGui::Text(ICON_FK_INFO_CIRCLE);
		break;
	case gte::internal::Logger::Type::WARNING:
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.9f, 0.0f, 1.0f });
		ImGui::Text(ICON_FK_EXCLAMATION_TRIANGLE);
		ImGui::PopStyleColor();
		break;
	case gte::internal::Logger::Type::ERR:
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.88f, 0.023f, 0.0f, 1.0f });
		ImGui::Text(ICON_FK_EXCLAMATION_CIRCLE);
		ImGui::PopStyleColor();
		break;
	default:
		ImGui::PopFont();
		return;
	}
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text(msg.c_str());
}