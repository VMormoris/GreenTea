#include "AnimationPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <gtc/matrix_transform.hpp>

void AnimationPanel::Draw(bool& show)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];
	auto BoldFont = io.Fonts->Fonts[1];

	if (!mHasBeenLoaded && mAnimationID.IsValid())
	{
		gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(mAnimationID);
		if (asset->Type == gte::AssetType::SPRITE_ANIMATION)
		{
			mAnimation = *((gte::internal::Animation*)asset->Data);
			mHasBeenLoaded = true;
		}
	}

	if (!show)
		return;
	ImGuiWindowClass window_class;
	window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&window_class);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("AnimationViewport", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();
	ImVec2 size = ImGui::GetContentRegionAvail();
	gte::internal::GetContext()->ViewportSize = { size.x, size.y };
	uint64 attachmentID = mFBO->GetColorAttachmentID(0);
	ImGui::Image((void*)attachmentID, size, { 0, 1 }, { 1, 0 });
	ImGui::End();

	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::Begin("##AnimationTools", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));
		const float btnSize = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x - btnSize) / 2.0f);
		ImGui::PushFont(IconsFont);
		if (ImGui::Button(ICON_FK_FLOPPY_O, { btnSize, btnSize }))
			mAnimation.Save(mAnimationID);
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	ImGui::Begin("Animation Properties", nullptr, ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoCollapse);
	const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	mViewportSize = { size.x, size.y };
	ImGui::PushFont(IconsFont);
	ImGui::Text(ICON_FK_KEY_MODERN);
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::Text(mAnimationID.str().c_str());
	ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 375.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f);
	if (ImGui::Button("Close"))
		show = false;
	ImGui::PopStyleVar();

	float height = 200.0f;
	float aspectRatio = 1.0f;
	float width = aspectRatio * height;
	float offset = (viewportSize.x - width) / 2.0f;
	gte::uuid textID = mAnimation.GetAtlas();
	gte::GPU::Texture* atlas = nullptr;
	if (textID.IsValid())
	{
		gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(textID);
		if (asset->Type == gte::AssetType::TEXTURE)
		{
			atlas = (gte::GPU::Texture*)asset->Data;
			aspectRatio = atlas->GetWidth() / (float)atlas->GetHeight();
			width = aspectRatio * height;
			if (width > viewportSize.x)
			{
				width = viewportSize.x;
				height = width / aspectRatio;
			}
			offset = (viewportSize.x - width) / 2.0f;
		}
	}
	const uint32 atlasWidth = atlas ? atlas->GetWidth() : 0;
	const uint32 atlasHeight = atlas ? atlas->GetHeight() : 0;

	ImGui::Dummy({0.0f, 0.0f});
	ImGui::SameLine(0.0f, offset);
	ImGui::Image(atlas == nullptr ? (void*)1 : atlas->GetID(), { width, height }, { 0, 1 }, { 1, 0 });
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			std::filesystem::path filepath = (const char*)payload->Data;
			if (filepath.extension() == ".gtimg")
				mAnimation.GetAtlas() = gte::internal::GetContext()->AssetWatcher.GetID(filepath.string());
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Separator();

	constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow;
	auto& frames = mAnimation.GetFrames();
	int32 index = -1;
	size_t one = 0, two = 0;
	for (size_t i = 0; i < frames.size(); i++)
	{
		const std::string label = "Frame " + std::to_string(i + 1);
		auto& frame = frames[i];
		//ImGui::InvisibleButton(("##" + label).c_str(), {-1.0f, 1.0f});
		//if (ImGui::BeginDragDropTarget())
		//{
		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATION_FRAME"))
		//	{
		//		one = *(size_t*)payload->Data;
		//		two = i;
		//	}
		//	ImGui::EndDragDropTarget();
		//}
		const bool open = ImGui::TreeNodeEx(label.c_str(), treeNodeFlags);
		if (ImGui::BeginDragDropSource())
		{
			ImGui::Text(label.c_str());
			ImGui::SetDragDropPayload("ANIMATION_FRAME", &i, sizeof(size_t));
			ImGui::EndDragDropSource();
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 24.0f);
		ImGui::PushFont(IconsFont);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		const bool remove = ImGui::Button(ICON_FK_TRASH);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopFont();
		
		if (open)
		{
			gte::gui::UISettings settings;
			gte::gui::DrawTextureCoordinates(frame.Coords, atlasWidth, atlasHeight, settings, false);
			settings.MaxFloat = FLT_MAX;
			gte::gui::DrawFloatControl("Duration", frame.Duration, settings);
			ImGui::TreePop();
			ImGui::Separator();
		}

		if (remove)
			index = static_cast<int32>(i);
	}
	//const std::string label = "Frame " + std::to_string(frames.size() - 1);
	//ImGui::InvisibleButton(("##" + label).c_str(), {-1.0f, 1.0f});
	//if (ImGui::BeginDragDropTarget())
	//{
	//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIMATION_FRAME"))
	//	{
	//		one = *(size_t*)payload->Data;
	//		two = frames.size() - 1;
	//	}
	//	ImGui::EndDragDropTarget();
	//}
	if (index >= 0)
		mAnimation.DeleteFrame(index);
	if (one != two)
		std::swap(frames[one], frames[two]);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	if (ImGui::Button("Add frame", { viewportSize.x, 0.0f }))
		auto& frame = frames.emplace_back();
	ImGui::PopStyleVar();
	ImGui::End();
}

void AnimationPanel::Update(float dt)
{
	constexpr float ClearColor = 30.0f / 255.0f;
	if (gte::GPU::FrameBufferSpecification spec = mFBO->GetSpecification();
		(mViewportSize.x > 0.0f) && (mViewportSize.y > 0.0f) &&
		((spec.Width != mViewportSize.x || spec.Height != mViewportSize.y)))
		mFBO->Resize(static_cast<uint32>(mViewportSize.x), static_cast<uint32>(mViewportSize.y));

	if (!mAnimationID.IsValid() || mAnimation.GetFrames().size() == 0)
		return;

	mAnimation.Update(dt);

	const auto& spec = mFBO->GetSpecification();
	gte::RenderCommand::SetViewport(0, 0, spec.Width, spec.Height);
	gte::RenderCommand::SetClearColor({ ClearColor, ClearColor, ClearColor, 1.0f });
	gte::Renderer2D::BeginFrame(mFBO);
	gte::Renderer2D::BeginScene(glm::mat4{1.0f});
	gte::uuid textID = mAnimation.GetAtlas();
	glm::mat4 model = glm::translate(glm::mat4{ 1.0f }, { 0.0f, 0.0f, 0.0f });
	if (!textID.IsValid())
		gte::Renderer2D::DrawQuad(model, (uint32)-1, { 1.0f, 1.0f, 1.0f, 1.0f });
	else
	{
		auto& frame = mAnimation.GetCurrentFrame();
		gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(textID);
		if (asset->Type == gte::AssetType::TEXTURE)
			gte::Renderer2D::DrawQuad(model, (gte::GPU::Texture*)asset->Data, frame.Coords, (uint32)-1);
		else
			gte::Renderer2D::DrawQuad(model, (uint32)-1, { 1.0f, 1.0f, 1.0f, 1.0f });
	}
	gte::Renderer2D::EndScene();
}

void AnimationPanel::SetAnimation(const gte::uuid& id)
{
	if (!id.IsValid())
	{
		mAnimationID = {};
		mAnimation = {};
		return;
	}

	mAnimationID = id;
	gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
	if (asset->Type == gte::AssetType::LOADING)
	{
		mAnimation = {};
		mHasBeenLoaded = false;
	}
	else
	{
		mAnimation = *((gte::internal::Animation*)asset->Data);
		mHasBeenLoaded = true;
	}
}

AnimationPanel::AnimationPanel(void)
{
	gte::GPU::FrameBufferSpecification spec;
	spec.Attachments = { { gte::GPU::TextureFormat::RGB8, gte::GPU::WrapFilter::CLAMP_EDGE, gte::GPU::ResizeFilter::LINEAR } };
	spec.Width = static_cast<uint32>(mViewportSize.x);
	spec.Height = static_cast<uint32>(mViewportSize.y);
	mFBO = gte::GPU::FrameBuffer::Create(spec);
}