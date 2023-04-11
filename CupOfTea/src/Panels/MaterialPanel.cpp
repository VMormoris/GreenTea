#include "MaterialPanel.h"
#include "EditorContext.h"

#include <imgui.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

void MaterialPanel::Draw(void)
{
	if (!mMaterialID.IsValid())
		return;

	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];
	auto BoldFont = io.Fonts->Fonts[1];

	ImGui::Begin("Material Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);

	void* imgid = nullptr;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
	gte::gui::UISettings settings;
	settings.ResetValue = 1.0f;
	
	DrawProperty("Albedo", mMaterial.Albedo, mMaterial.Diffuse, "Define the base color of the Material, which tints or recolors the Material.\nUse the color picker to set the base color of the rendered Material.");
	DrawProperty("Normal", mMaterial.Normal);
	DrawProperty("Metallicness", mMaterial.Metallic, mMaterial.Metallicness);
	DrawProperty("Roughness", mMaterial.Rough, mMaterial.Roughness);
	DrawProperty("Occlusion", mMaterial.AmbientOclussion, mMaterial.AmbientColor, "Define the ambient color of the Material, which tints or recolors the Material.\nUse the color picker to set the ambient color of the rendered Material.");
	DrawProperty("Opacity", mMaterial.Opacity, mMaterial.Alpha);
	DrawProperty("Emission", mMaterial.Emission, mMaterial.EmitColor, "Define the emissive color of the Material, which tints or recolors the Material.\nUse the color picker to set the emissive color of the rendered Material.", 0.0f);

	ImGui::Dummy({ 1.0f, 3.0f });

	constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow;
	if (ImGui::TreeNodeEx("Preview", treeNodeFlags))
	{
		let size = ImGui::GetWindowContentRegionMax().x;
		let imgsize = ImGui::GetContentRegionAvail().x - 96.0f;
		let offset = (size - imgsize) / 2.0f;
		ImGui::SetCursorPosX(offset);
		gte::ThumbnailRenderer::Render(GetEditorContext()->SphereGeometry, mMaterial, glm::ivec2 { (int32)imgsize }, mRotation);
		let* fbo = gte::ThumbnailRenderer::GetThumbnail();
		ImGui::Image((void*)fbo->GetColorAttachmentID(0), { imgsize, imgsize }, { 0, 1 }, { 1, 0 });
		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			constexpr float speed = 0.003f;
			let delta = glm::vec2{ io.MouseDelta.x, io.MouseDelta.y } * speed;
			
			let orientation = glm::quat(glm::radians(mRotation));
			let angleX = glm::angleAxis(delta.x, glm::vec3{0.0f, 1.0f, 0.0f});
			let angleY = glm::angleAxis(delta.y, glm::vec3{ 1.0f, 0.0f, 0.0f });
			let rotation = angleY * angleX * orientation;

			mRotation = glm::degrees(glm::eulerAngles(rotation));
		}
		ImGui::TreePop();
	}
	ImGui::PopStyleVar();

	let pos = ImVec2{ ImGui::GetContentRegionAvail().x - 140.0f, ImGui::GetContentRegionMax().y - 24.0f };
	ImGui::SetCursorPos(pos);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	if (ImGui::Button("Close", { 72.0f, 0.0f }))
		mMaterialID = {};
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.079f, 0.538f, 0.902f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.179f, 0.638f, 1.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.039f, 0.498f, 0.862f, 1.0f });
	if (ImGui::Button("Save", { 72.0f, 0.0f }))
	{
		SaveMaterial();
		mMaterialID = {};
	}

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar();

	ImGui::End();
}

void MaterialPanel::SetMaterial(const gte::uuid& id)
{
	mRotation = { 0.0f, 0.0f, 0.0f };
	if (!id.IsValid())
	{
		mMaterialID = {};
		mMaterial = {};
		return;
	}

	mMaterialID = id;
	gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
	while(asset->Type == gte::AssetType::LOADING)
		asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
	mMaterial = *(gte::Material*)asset->Data;
}

void MaterialPanel::DrawImageControl(gte::Ref<gte::Asset>& asset)
{
	ImGui::SetCursorPos({ 6.0f, ImGui::GetCursorPosY() + 3.0f });
	void* imgid = GetEditorContext()->TransparencyGrid->GetID();
	asset = gte::internal::GetContext()->AssetManager.RequestAsset(asset->ID);
	if (let text = asset->Data)
		imgid = ((gte::GPU::Texture*)text)->GetID();
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			asset->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
		ImGui::EndDragDropTarget();
	}
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##Texture Control");
	if (ImGui::BeginPopup("##Texture Control"))
	{
		if (ImGui::MenuItem("Remove", nullptr, nullptr, imgid))
			asset->ID = {};
		ImGui::EndPopup();
	}
}

void MaterialPanel::DrawProperty(const char* label, gte::Ref<gte::Asset>& asset)
{
	ImGui::PushID(label);
	DrawImageControl(asset);
	ImGui::SameLine();
	let y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text(label);
	ImGui::SetCursorPosY(y);
	ImGui::Dummy({ 26.0f, 26.0f });
	ImGui::PopID();
}

void MaterialPanel::DrawProperty(const char* label, gte::Ref<gte::Asset>& asset, float& value, const char* help)
{
	ImGui::PushID(label);
	DrawImageControl(asset);
	ImGui::SameLine();
	let y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
	ImGui::Text(label); ImGui::SameLine(); ImGui::Text(":");
	ImGui::PopStyleVar();
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.0f, y });
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 3.0f);
	ImGui::SliderFloat("##", &value, 0.0f, 1.0f);
	ImGui::PopItemWidth();
	ImGui::PopID();
}

void MaterialPanel::DrawProperty(const char* label, gte::Ref<gte::Asset>& asset, glm::vec3& color, const char* help, float resetVal)
{
	gte::gui::UISettings settings;
	settings.ResetValue = resetVal;
	ImGui::PushID(label);
	DrawImageControl(asset);
	ImGui::SameLine();
	let y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
	ImGui::Text(label); ImGui::SameLine(); ImGui::Text(":");
	ImGui::PopStyleVar();
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.5f, y });
	gte::gui::DrawColorPicker(color, settings, help);
	ImGui::PopID();
}

void MaterialPanel::SaveMaterial(void)
{
	using namespace gte::math;
	gte::ThumbnailRenderer::Render(GetEditorContext()->SphereGeometry, mMaterial);
	let* fbo = gte::ThumbnailRenderer::GetThumbnail();

	constexpr uint32 width = 128;
	constexpr uint32 height = 128;
	constexpr uint32 bpp = 4;
	gte::Image img(width, height, 4);
	fbo->ReadPixels(0, img.Data());

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Name" << YAML::Value << mMaterial.Name;
	out << YAML::Key << "Albedo" << YAML::Value << mMaterial.Albedo->ID.str();
	out << YAML::Key << "Metallic" << YAML::Value << mMaterial.Metallic->ID.str();
	out << YAML::Key << "Rough" << YAML::Value << mMaterial.Rough->ID.str();
	out << YAML::Key << "Normal" << YAML::Value << mMaterial.Normal->ID.str();
	out << YAML::Key << "AmbientOclussion" << YAML::Value << mMaterial.AmbientOclussion->ID.str();
	out << YAML::Key << "Opacity" << YAML::Value << mMaterial.Opacity->ID.str();
	out << YAML::Key << "Emission" << YAML::Value << mMaterial.Emission->ID.str();

	out << YAML::Key << "Diffuse" << YAML::Value << mMaterial.Diffuse;
	out << YAML::Key << "EmitColor" << YAML::Value << mMaterial.EmitColor;
	out << YAML::Key << "AmbientColor" << YAML::Value << mMaterial.AmbientColor;
	out << YAML::Key << "Metallicness" << YAML::Value << mMaterial.Metallicness;
	out << YAML::Key << "Roughness" << YAML::Value << mMaterial.Roughness;
	out << YAML::Key << "Alpha" << YAML::Value << mMaterial.Alpha;
	out << YAML::Key << "IlluminationModel" << YAML::Value << mMaterial.IlluminationModel;
	out << YAML::Key << "IsEmissive" << YAML::Value << mMaterial.IsEmissive;
	out << YAML::Key << "Thumbnail" << YAML::Value << img.Size();
	out << YAML::EndMap;

	let filepath = gte::internal::GetContext()->AssetWatcher.GetFilepath(mMaterialID);
	std::time_t result = std::time(nullptr);
	std::ofstream os(filepath, std::ios::binary);
	os << "# Material for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 13 << '\n' << mMaterialID << '\n' << out.size() << "\n\n";
	os << out.c_str();
	os.write((char*)&width, 4);
	os.write((char*)&height, 4);
	os.write((char*)&bpp, 4);
	os.write((char*)img.Data(), img.Size());
	os.close();

	gte::internal::GetContext()->AssetManager.CreateThumbnail(mMaterialID, img);
}
