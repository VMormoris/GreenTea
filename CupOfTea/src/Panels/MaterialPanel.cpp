#include "MaterialPanel.h"
#include "EditorContext.h"

#include <imgui.h>

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
	ImGui::PushID("Albedo");
	ImGui::SetCursorPosX(6.0f);
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.Albedo = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.Albedo->ID);
	if (let albedo = mMaterial.Albedo->Data)
		imgid = ((gte::GPU::Texture*)albedo)->GetID();
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			mMaterial.Albedo->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
			mMaterial.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	float y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Color:");
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.5f, y });
	gte::gui::DrawColorPicker(mMaterial.Diffuse, settings, "Define the base color of the Material, which tints or recolors the Material.\nUse the color picker to set the base color of the rendered Material.");
	ImGui::PopID();

	ImGui::PushID("Normal");
	ImGui::SetCursorPos({ 6.0f, ImGui::GetCursorPosY() + 3.0f });
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.Normal = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.Normal->ID);
	if (let normal = mMaterial.Normal->Data)
		imgid = ((gte::GPU::Texture*)normal)->GetID();
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			mMaterial.Normal->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Normal");
	ImGui::SetCursorPosY(y);
	ImGui::Dummy({ 26.0f, 26.0f });
	ImGui::PopID();

	ImGui::PushID("Metallic");
	ImGui::SetCursorPosX(6.0f);
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.Metallic = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.Metallic->ID);
	if (let metallic = mMaterial.Metallic->Data)
		imgid = ((gte::GPU::Texture*)metallic)->GetID();
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			mMaterial.Metallic->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
			mMaterial.Metallicness = 1.0f;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Matallicness:");
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.0f, y });
	ImGui::SliderFloat("##", &mMaterial.Metallicness, 0.0f, 1.0f);
	ImGui::PopID();

	ImGui::PushID("Rough");
	ImGui::SetCursorPosX(6.0f);
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.Rough = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.Rough->ID);
	if (let rougness = mMaterial.Rough->Data)
		imgid = ((gte::GPU::Texture*)rougness)->GetID();
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			mMaterial.Rough->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
			mMaterial.Roughness = 1.0f;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Roughness:");
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.0f, y });
	ImGui::SliderFloat("##", &mMaterial.Roughness, 0.0f, 1.0f);
	ImGui::PopID();

	ImGui::PushID("Ambient");
	ImGui::SetCursorPosX(6.0f);
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.AmbientOclussion = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.AmbientOclussion->ID);
	if (let occlusion = mMaterial.AmbientOclussion->Data)
		imgid = ((gte::GPU::Texture*)occlusion)->GetID();
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			mMaterial.AmbientOclussion->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Occlusion:");
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.5f, y });
	gte::gui::DrawColorPicker(mMaterial.AmbientColor, settings, "Define the ambient color of the Material, which tints or recolors the Material.\nUse the color picker to set the ambient color of the rendered Material.");
	ImGui::PopID();

	ImGui::PushID("Opacity");
	ImGui::SetCursorPos({ 6.0f, ImGui::GetCursorPosY() + 3.0f });
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.Opacity = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.Opacity->ID);
	ImGui::Image(imgid, { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			mMaterial.Opacity->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
			mMaterial.Alpha = 1.0f;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Opacity:");
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.0f, y });
	ImGui::SliderFloat("##", &mMaterial.Alpha, 0.0f, 1.0f);
	ImGui::PopID();

	ImGui::PushID("Emission");
	ImGui::SetCursorPosX(6.0f);
	imgid = GetEditorContext()->TransparencyGrid->GetID();
	mMaterial.Emission = gte::internal::GetContext()->AssetManager.RequestAsset(mMaterial.Emission->ID);
	if (let emissive = mMaterial.Emission->Data)
		imgid = ((gte::GPU::Texture*)emissive)->GetID();
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 26.0f, 26.0f }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (let* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			mMaterial.Emission->ID = gte::internal::GetContext()->AssetWatcher.GetID((const char*)payload->Data);
			mMaterial.EmitColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	y = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
	ImGui::Text("Emission:");
	ImGui::SameLine();
	ImGui::SetCursorPos({ 139.5f, y });
	gte::gui::DrawColorPicker(mMaterial.EmitColor, settings, "Define the emissive color of the Material, which tints or recolors the Material.\nUse the color picker to set the emissive color of the rendered Material.");
	ImGui::PopID();

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
			constexpr float speed = 0.33f;
			let delta = io.MouseDelta;
			mRotation.x += delta.y * speed;
			mRotation.y += delta.x * speed;
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
	ImGui::Button("Save", { 72.0f, 0.0f });
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