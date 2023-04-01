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

	ImGui::Begin("Material Editor", nullptr/*, ImGuiWindowFlags_NoResize*/);
	constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");

	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");

	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");


	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");

	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");

	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");

	ImGui::SetCursorPosX(6.0f);
	ImGui::Image(GetEditorContext()->TransparencyGrid->GetID(), { 24.0f, 24.0f }, { 0, 1 }, { 1, 0 });
	//gte::gui::DrawColorPicker("", mMaterial.Diffuse, settings, "Set the diffuse color");

	if (ImGui::TreeNodeEx("Preview", treeNodeFlags))
	{
		let size = ImGui::GetWindowContentRegionMax().x;
		let imgsize = ImGui::GetContentRegionAvail().x - 64.0f;
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
	mMaterial = *(gte::Material*)asset->Data;
}