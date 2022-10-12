#include "CupOfTea.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <IconsForkAwesome.h>
#include <ImGuizmo.h>
#include <gtc/matrix_transform.hpp>

#include <fstream>
#include <Engine/Renderer/Renderer2D.h>

static gte::GPU::Texture* sIcon = nullptr;
static bool sLoaded = false;
static ImGuizmo::OPERATION sGuizmoOP = ImGuizmo::OPERATION::BOUNDS;
static gte::GPU::FrameBuffer* sCamFBO = nullptr;
static glm::vec2 sCamViewport;
static constexpr entt::entity EnttNull = entt::null;
static bool VisualizeOperation(void);
static bool sGameEvents = true;

void CupOfTea::Update(float dt)
{
	constexpr float ClearColor = 30.0f / 255.0f;
	gte::Scene* scene = gte::internal::GetContext()->ActiveScene;

	if (sGameEvents && !gte::internal::GetContext()->Playing)
	{
		gte::Entity EditorCamera = scene->FindEntityWithUUID({});
		auto& tc = EditorCamera.GetComponent<gte::Transform2DComponent>();
		const auto& settings = EditorCamera.GetComponent<gte::Settings>();
		bool changed = false;
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::W))
		{
			tc.Position.y += settings.CameraVelocity.y * dt;
			changed = true;
		}
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::S))
		{
			tc.Position.y -= settings.CameraVelocity.y * dt;
			changed = true;
		}
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::D))
		{
			tc.Position.x += settings.CameraVelocity.x * dt;
			changed = true;
		}
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::A))
		{
			tc.Position.x -= settings.CameraVelocity.x * dt;
			changed = true;
		}
		if (changed)
			scene->UpdateTransform(EditorCamera);
	}

    Application::Update(dt);
    gte::Window* window = gte::internal::GetContext()->GlobalWindow;
	glm::vec2& viewportSize = gte::internal::GetContext()->ViewportSize;
	if (gte::GPU::FrameBufferSpecification spec = mViewportFBO->GetSpecification();
		(viewportSize.x > 0.0f) && (viewportSize.y > 0.0f) &&
		((spec.Width != viewportSize.x || spec.Height != viewportSize.y)))
	{
		mViewportFBO->Resize((uint32)viewportSize.x, (uint32)viewportSize.y);
		gte::GPU::FrameBufferSpecification newSpec = mViewportFBO->GetSpecification();
		gte::RenderCommand::SetViewport(0, 0, newSpec.Width, newSpec.Height);
		scene->OnViewportResize(newSpec.Width, newSpec.Height);
	}

	mViewportFBO->Bind();
    gte::RenderCommand::SetClearColor({ ClearColor, ClearColor, ClearColor, 1.0f });
    gte::RenderCommand::Clear();
	mViewportFBO->Clear(1, &EnttNull);

	if (gte::internal::GetContext()->AssetWatcher.Reload())
	{
		const auto& changes = gte::internal::GetContext()->AssetWatcher.GetChanges();
		for (const auto& id : changes)
			gte::internal::GetContext()->AssetManager.RemoveAsset(id);
		sLoaded = false;
	}
	if (gte::internal::GetContext()->AssetWatcher.IsBuilding() && gte::internal::GetContext()->Playing)
		Stop();

	if (gte::internal::GetContext()->Playing)
		scene->Update(dt);
	else
		scene->UpdateEditor();
	OnOverlayRenderer();
	mViewportFBO->Unbind();

	gte::Entity entity = mSceneHierarchyPanel.GetSelectedEntity();
	const bool playing = gte::internal::GetContext()->Playing;
	if (!playing && entity && entity.HasComponent<gte::CameraComponent>() && sCamViewport.x > 0.0f && sCamViewport.y > 0.0f)
	{
		const auto& ortho = entity.GetComponent<gte::OrthographicCameraComponent>();
		auto& cam = entity.GetComponent<gte::CameraComponent>();
		if (!cam.FixedAspectRatio)
		{
			cam.AspectRatio = sCamViewport.x / sCamViewport.y;
			glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
			box *= glm::vec2(cam.AspectRatio, 1.0f);

			cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}

		sCamFBO->Resize(static_cast<uint32>(sCamViewport.x), static_cast<uint32>(sCamViewport.y));
		sCamFBO->Bind();
		gte::RenderCommand::SetViewport(0, 0, static_cast<uint32>(sCamViewport.x), static_cast<uint32>(sCamViewport.y));
		gte::RenderCommand::SetClearColor({ ClearColor, ClearColor, ClearColor, 1.0f });
		gte::RenderCommand::Clear();
		scene->Render(cam.EyeMatrix);
		sCamFBO->Unbind();
		
		gte::RenderCommand::SetViewport(0, 0, static_cast<uint32>(viewportSize.x), static_cast<uint32>(viewportSize.y));
	}

	RenderGUI();
}

void CupOfTea::OnOverlayRenderer(void)
{
	gte::Scene* scene = gte::internal::GetContext()->ActiveScene;
	auto view = scene->GetAllEntitiesWith<gte::TransformationComponent, gte::CircleColliderComponent>();

	glm::mat4 eyeMatrix;
	if (!gte::internal::GetContext()->Playing)
	{
		gte::Entity entity = scene->FindEntityWithUUID({});
		eyeMatrix = entity.GetComponent<gte::CameraComponent>().EyeMatrix;
	}
	else
	{
		auto cams = scene->GetAllEntitiesWith<gte::CameraComponent>();
		bool found = false;
		for (auto&& [entityID, cam] : cams.each())
		{
			if (cam.Primary)
			{
				eyeMatrix = cam.EyeMatrix;
				found = true;
				break;
			}
		}
		if (!found)
			return;
	}

	gte::Renderer2D::BeginScene(eyeMatrix);
	if (mShowColliders)
	{
		auto boxes = scene->GetAllEntitiesWith<gte::TransformationComponent, gte::BoxColliderComponent>();
		for (auto&& [entityID, tc, bc] : boxes.each())
		{
			glm::vec3 pos, scale, rotation;
			gte::math::DecomposeTransform(tc, pos, scale, rotation);

			pos += glm::vec3{bc.Offset, 0.0f};
			scale = scale * glm::vec3{ bc.Size * 2.0f, 1.0f };
			
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), rotation.z, { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), {scale.x, scale.y, 1.0f});
			gte::Renderer2D::DrawRect(transform, { 0.0f, 1.0f, 0.0f, 1.0f });
		}

		auto circles = scene->GetAllEntitiesWith<gte::TransformationComponent, gte::CircleColliderComponent>();
		for (auto&& [entityID, tc, cc] : circles.each())
		{
			glm::vec3 pos, scale, rotation;
			gte::math::DecomposeTransform(tc, pos, scale, rotation);

			pos += glm::vec3{ cc.Offset, 0.0f };
			scale = scale * glm::vec3{ glm::vec2(cc.Radius) * 2.0f, 1.0f };
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });
			gte::Renderer2D::DrawCircle(transform, { 0.0f, 1.0f, 0.0f, 1.0f }, (uint32)EnttNull, 0.1f);
		}
	}

	if (gte::Entity entity = mSceneHierarchyPanel.GetSelectedEntity())
	{
		if (!gte::internal::GetContext()->Playing && entity.HasComponent<gte::TransformationComponent>())
			gte::Renderer2D::DrawRect(entity.GetComponent<gte::TransformationComponent>(), { 1.0f, 0.5f, 0.0f, 1.0f });
	}
	gte::Renderer2D::EndScene();

}

void CupOfTea::RenderGUI(void)
{
    gui->Begin();
    ImGui::SetCurrentContext(gui->GetContext());
	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];
	auto BoldFont = io.Fonts->Fonts[1];

	//DockSpace From ImGui Demo
	// Note: Switch this to true to enable dockspace
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		ImGuiDockNode* Node = ImGui::DockBuilderGetNode(dockspace_id);
		Node->SharedFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;
	}

	if (ImGui::BeginMainMenuBar())
	{
		//ImGui::Image(sIcon->GetID(), { 64.0f, 51.0f }, { 0, 1 }, { 1, 0 });
		if (ImGui::BeginMenu("File"))
		{
			const char biggest[] = "Save Scene As...Ctrl+Shift+N";
			if (gte::gui::DrawMenuItem(ICON_FK_CUBES, "New Scene", "Ctrl+N", biggest))
				NewScene();
			if (gte::gui::DrawMenuItem(ICON_FK_BOOK, "New Project", "Ctrl+Shift+N", biggest))
			{
			}
			if (gte::gui::DrawMenuItem(ICON_FK_FOLDER_OPEN, "Open Project...", "Ctrl+O", biggest))
			{
				
			}
			if (gte::gui::DrawMenuItem(ICON_FK_FLOPPY_O, "Save Scene", "Ctrl+S", biggest))
				SaveScene();
			if (gte::gui::DrawMenuItem(ICON_FK_FILE, "Save Scene As...", "Ctrl+Shift+S", biggest))
				SaveSceneAs(gte::internal::CreateFileDialog(gte::internal::FileDialogType::Save, "Green Tea Scene (*.gtscene)\0*.gtscene\0"));
			ImGui::Separator();
			if (gte::gui::DrawMenuItem(ICON_FK_POWER_OFF, "Exit", "Alt+F4", biggest))
				Close();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Viewport", nullptr, &mPanels[0]);
			ImGui::MenuItem("Content Browser", nullptr, &mPanels[1]);
			ImGui::MenuItem("Hierarchy", nullptr, &mPanels[2]);
			ImGui::MenuItem("Properties", nullptr, &mPanels[3]);
			ImGui::MenuItem("Console Log", nullptr, &mPanels[4]);
			ImGui::EndMenu();
		}

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		if (ImGui::Button("About"))
			mPanels[5] = !mPanels[5];
		ImGui::PopStyleColor();

		ImGui::EndMainMenuBar();
	}
	ImGui::End();//End Dockspace
	
	if (mPanels[0])
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		if (ImGui::Begin("Viewport", &mPanels[0]))
		{
			sGameEvents = ImGui::IsWindowFocused();
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 size = ImGui::GetContentRegionAvail();
			gte::internal::GetContext()->ViewportSize = { size.x, size.y };
			uint64 textID = mViewportFBO->GetColorAttachmentID(0);
			ImGui::Image((void*)textID, size, { 0, 1 }, { 1, 0 });
			bool selection = ImGui::IsMouseReleased(ImGuiPopupFlags_MouseButtonLeft);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path filepath = (const char*)payload->Data;
					if (filepath.extension() == ".gtscene")
						OpenScene(filepath);
				}
				ImGui::EndDragDropTarget();
			}

			if(VisualizeOperation())
				selection = false;
			if (DrawGuizmo())
				selection = false;

			if (selection)
			{
				auto [x, y] = ImGui::GetMousePos();
				x -= ImGui::GetWindowPos().x;
				y -= ImGui::GetWindowPos().y;
				y = windowSize.y - y;
				entt::entity enttID = entt::null;
				mViewportFBO->GetPixel(1, static_cast<int32>(x), static_cast<int32>(y), &enttID);
				if (enttID != entt::null)
					mSceneHierarchyPanel.SetSelectedEntity({ enttID, gte::internal::GetContext()->ActiveScene });
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::Begin("##Tools", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	auto& colors = ImGui::GetStyle().Colors;
	const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
	const auto& buttonActive = colors[ImGuiCol_ButtonActive];
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));
	const float btnSize = ImGui::GetWindowHeight() - 4.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x - btnSize) / 2.0f);
	ImGui::PushFont(IconsFont);
	const bool building = gte::internal::GetContext()->AssetWatcher.IsBuilding();
	if (building)
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	if (ImGui::Button(gte::internal::GetContext()->Playing ? ICON_FK_STOP : ICON_FK_PLAY, { btnSize, btnSize }))
	{
		if (gte::internal::GetContext()->Playing)//Stop
			Stop();
		else//Start
			Start();
	}
	if (building)
		ImGui::PopItemFlag();
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);
	ImGui::End();
	ImGui::PopStyleVar(2);

	if (mPanels[2])
	{
		if (ImGui::Begin("Scene Hierarchy", &mPanels[2], ImGuiWindowFlags_NoCollapse))
		{
			mSceneHierarchyPanel.SetDirectory(mBrowserPanel.GetCurrentPath());
			mSceneHierarchyPanel.Draw();
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && gte::Input::IsKeyPressed(gte::KeyCode::DEL))
				mSceneHierarchyPanel.DeleteSelected();
		}
		ImGui::End();
	}

	if (mPanels[3])
	{
		if (ImGui::Begin("Properties", &mPanels[3], ImGuiWindowFlags_NoCollapse))
			mSceneHierarchyPanel.DrawComponents(mSceneHierarchyPanel.GetSelectedEntity());
		ImGui::End();
	}

	if (mPanels[4])
	{
		if (ImGui::Begin("Console Log", &mPanels[4], ImGuiWindowFlags_NoCollapse))
			mConsolePanel.Draw();
		ImGui::End();
	}

	if (mPanels[1])
	{
		if (ImGui::Begin("Content Broswer", &mPanels[1], ImGuiWindowFlags_NoCollapse))
		{
			mBrowserPanel.Draw();
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && gte::Input::IsKeyPressed(gte::KeyCode::DEL))
				mBrowserPanel.DeleteSelected();
		}
		ImGui::End();
	}

	if (mPanels[5])
	{
		if (ImGui::Begin("About##popup", &mPanels[5], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrapped("GreenTea is 2D Game Engine that is heavily inspired by Cherno's Game Engine Series. The engine is currently only being developed by me, on my free time, with the only purpose of helping me understand better how game engines as well as game development works.\n\n");
			ImGui::PushFont(BoldFont);
			ImGui::Text("Third Party Libraries");
			ImGui::PopFont();
			ImGui::BulletText("box2d");
			ImGui::BulletText("Dear ImGui");
			ImGui::BulletText("entt");
			ImGui::BulletText("glad");
			ImGui::BulletText("GLFW");
			ImGui::BulletText("glm");
			ImGui::BulletText("IconFontCppHeaders");
			ImGui::BulletText("ImGuizmo");
			ImGui::BulletText("libsndfile");
			ImGui::BulletText("openal-soft");
			ImGui::BulletText("premake5");
			ImGui::BulletText("stb");
			ImGui::BulletText("yaml-cpp");

			ImGui::PushFont(BoldFont);
			ImGui::Text("\nAssets");
			ImGui::PopFont();
			ImGui::BulletText("Fork Awesome");
			ImGui::BulletText("Open Sans");
			ImGui::BulletText("Papirus icon theme");
		}
		ImGui::End();
	}

	if (mPanels[6])
	{
		if (ImGui::Begin("Settings", &mPanels[6], ImGuiWindowFlags_NoCollapse))
		{
			gte::Entity SceneEntity = gte::internal::GetContext()->ActiveScene->FindEntityWithUUID({});
			//auto& settings = SceneEntity.GetComponent<gte::Settings>();
			gte::gui::DrawComponent<gte::Settings>(ICON_FK_GLOBE, "Physics Engine", SceneEntity, [&](auto& settings) {
				gte::gui::UISettings guiSettings;
				gte::gui::DrawVec2Control("Gravity", settings.Gravity, guiSettings);
				guiSettings.MinFloat = 5.0f;
				guiSettings.MaxFloat = FLT_MAX;
				gte::gui::DrawInt32Control("Tick Rate", settings.Rate, guiSettings);
				guiSettings.MinFloat = 3.0f;
				gte::gui::DrawInt32Control("Velocity Iterations", settings.VelocityIterations, guiSettings);
				guiSettings.MinFloat = 1.0f;
				gte::gui::DrawInt32Control("Postion Iterations", settings.PositionIterations, guiSettings);
				gte::gui::DrawBoolControl("Show Colliders", mShowColliders, guiSettings);
			});
			
			gte::gui::DrawComponent<gte::CameraComponent>(ICON_FK_CAMERA_RETRO, "Editor's Camers", SceneEntity, [&](auto& cam) {
				gte::gui::UISettings settings;
				auto& tc = SceneEntity.GetComponent<gte::Transform2DComponent>();
				glm::vec2 pos = { tc.Position.x, tc.Position.y };
				if (gte::gui::DrawVec2Control("Position", pos, settings))
				{
					tc.Position = { pos.x, pos.y, 0.0f };
					auto& transorm = glm::translate(glm::mat4(1.0f), tc.Position);
					cam.ViewMatrix = glm::inverse(transorm);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
				}

				auto& Velocity = SceneEntity.GetComponent<gte::Settings>().CameraVelocity;
				settings.MinFloat = 0.25f;
				settings.MaxFloat = FLT_MAX;
				gte::gui::DrawVec2Control("Velocity", Velocity, settings);

				auto& ortho = SceneEntity.GetComponent<gte::OrthographicCameraComponent>();
				settings.MinFloat = 0.1f;
				const bool zoom = gte::gui::DrawFloatControl("Zoom Level", ortho.ZoomLevel, settings);
				settings.MinFloat = 0.5f;
				const bool boundary = gte::gui::DrawFloatControl("Vertical Boundary", ortho.VerticalBoundary, settings);
				if (zoom || boundary)
				{
					glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
					box *= glm::vec2(cam.AspectRatio, 1.0f);
					cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
				}
			});
			
		}
		ImGui::End();
	}

	gte::Entity entity = mSceneHierarchyPanel.GetSelectedEntity();
	if (!gte::internal::GetContext()->Playing && entity && entity.HasComponent<gte::CameraComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		if (ImGui::Begin("Camera POV", NULL, ImGuiWindowFlags_NoCollapse))
		{
			ImVec2 size = ImGui::GetContentRegionAvail();
			sCamViewport = { size.x, size.y };
			uint64 textID = sCamFBO->GetColorAttachmentID(0);
			ImGui::Image((void*)textID, size, { 0.0f, 1.0f }, { 1.0f, 0.0f });
		}
		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}

	if (!sLoaded)
	{
		ImGui::OpenPopup("Loading");

		ImVec2 size = { 160.0f, 32.0f };
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 pos = { (viewport->Size.x - size.x) / 2.0f, (viewport->Size.y - size.y) / 2.0f };
		ImGui::SetNextWindowPos({ viewport->Pos.x + pos.x, viewport->Pos.y + pos.y });
		ImGui::SetNextWindowSize(size);
	}

	if (ImGui::BeginPopupModal("Loading", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
	{
		ImGui::Text("Loading assets...");
		std::vector<gte::uuid> ids = gte::internal::GetContext()->AssetWatcher.GetAssets({ ".gtscript" });
		sLoaded = true;
		for (const auto& id : ids)
		{
			gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
			if (asset->Type == gte::AssetType::LOADING)
				sLoaded = false;
		}
		if (sLoaded)
		{
			gte::internal::GetContext()->ActiveScene->PatchScripts();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	//ImGui::ShowDemoWindow();
    gui->End();
}

CupOfTea::CupOfTea(const std::string& filepath)
	: Application({"CupOfTea (Green Tea editor)", 0, 0, 1080, 720, false, true, true}), mBrowserPanel("Assets")
{
    gte::Window* window = gte::internal::GetContext()->GlobalWindow;
    gui = gte::gui::ImGuiLayer::Create();
    gui->Init(window->GetPlatformWindow(), window->GetContext());

	gte::Image img("../Assets/Icons/GreenTea.png");
	sIcon = gte::GPU::Texture2D::Create(img);

	std::filesystem::path prjdir = filepath;
	prjdir = prjdir.parent_path();
	std::filesystem::current_path(prjdir);

	std::string scenename;
	std::ifstream is(filepath);
	std::getline(is, scenename);
	is.close();

	gte::internal::GetContext()->ActiveScene = new gte::Scene();
	gte::internal::GetContext()->AssetWatcher.LoadProject(".");
	if (std::filesystem::exists("Assets/" + scenename))
	{
		std::vector<gte::uuid> ids = gte::internal::GetContext()->AssetWatcher.GetAssets({ ".gtscript" });
		while (true)
		{
			bool loaded = true;
			for (const auto& id : ids)
			{
				gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
				if (asset->Type == gte::AssetType::LOADING)
					loaded = false;
			}
			if (loaded)
				break;
		}
		OpenScene("Assets/" + scenename);
	}
	
	auto solutionFile = prjdir.filename().string() + ".sln";
	ShellExecuteA(0, 0, solutionFile.c_str(), 0, 0, SW_SHOW);
	gte::internal::GetContext()->ActiveScene->OnViewportResize(window->GetWidth(), window->GetHeight());
	gte::internal::GetContext()->ScriptEngine = new gte::internal::ScriptingEngine();

	gte::GPU::FrameBufferSpecification spec;
	spec.Attachments = { gte::GPU::TextureFormat::RGB8, gte::GPU::TextureFormat::Int32 };
	spec.Width = window->GetWidth();
	spec.Height = window->GetHeight();
	mViewportFBO = gte::GPU::FrameBuffer::Create(spec);
	spec.Attachments = { gte::GPU::TextureFormat::RGB8 };
	sCamFBO = gte::GPU::FrameBuffer::Create(spec);

	gte::internal::GetContext()->ViewportSize = { static_cast<float>(spec.Width), static_cast<float>(spec.Height) };
	sCamViewport = { static_cast<float>(spec.Width), static_cast<float>(spec.Height) };

	REGISTER(gte::EventType::KeyPressed, this, &CupOfTea::OnKeyDown);
	REGISTER(gte::EventType::MouseScroll, this, &CupOfTea::OnScroll);
}

CupOfTea::~CupOfTea(void)
{
	UNREGISTER(this);
	delete sIcon;
    gui->Shutdown();
	delete mViewportFBO;
	delete sCamFBO;
    delete gui;
}

void CupOfTea::NewScene(void)
{
	delete gte::internal::GetContext()->ActiveScene;
	gte::internal::GetContext()->ActiveScene = new gte::Scene();
	mSceneHierarchyPanel.SetSelectedEntity({});
	mScenePath = "";

	glm::vec2 size = gte::internal::GetContext()->ViewportSize;
	gte::internal::GetContext()->ActiveScene->OnViewportResize(static_cast<uint32>(size.x), static_cast<uint32>(size.y));
}

void CupOfTea::OpenScene(const std::filesystem::path& path)
{
	mSceneHierarchyPanel.SetSelectedEntity({});
	delete gte::internal::GetContext()->ActiveScene;
	gte::internal::GetContext()->ActiveScene = new gte::Scene();
	gte::internal::SceneSerializer serializer(gte::internal::GetContext()->ActiveScene);
	serializer.Deserialize(path.string());
	mScenePath = path.string();

	glm::vec2 size = gte::internal::GetContext()->ViewportSize;
	gte::internal::GetContext()->ActiveScene->OnViewportResize(static_cast<uint32>(size.x), static_cast<uint32>(size.y));
	
	auto gtFile = std::filesystem::current_path().filename().string() + ".gt";
	auto lastScene = std::filesystem::relative(path, std::filesystem::current_path() / "Assets");
	auto output = lastScene.string();
	std::replace(output.begin(), output.end(), '\\', '/');
	std::ofstream os(std::filesystem::current_path() / gtFile);
	os << output;
	os.close();
}

void CupOfTea::SaveScene(void)
{
	if (mScenePath.empty())
		SaveSceneAs(gte::internal::CreateFileDialog(gte::internal::FileDialogType::Save, "Green Tea Scene (*.gtscene)\0*.gtscene\0"));
	else
		SaveSceneAs(mScenePath);
}

void CupOfTea::SaveSceneAs(const std::filesystem::path& path)
{
	auto filepath = path;
	filepath.replace_extension("gtscene");
	gte::internal::SceneSerializer serializer(gte::internal::GetContext()->ActiveScene);
	serializer.Serialize(filepath.string());
}

bool CupOfTea::OnKeyDown(gte::KeyCode keycode)
{
	const bool playing = gte::internal::GetContext()->Playing;
	const bool ctrl = gte::Input::IsKeyPressed(gte::KeyCode::LEFT_CONTROL);
	const bool shift = gte::Input::IsKeyPressed(gte::KeyCode::LEFT_SHIFT);
	switch (keycode)
	{
	case gte::KeyCode::Q:
		if (playing || !sGameEvents || gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right)) return false;
		sGuizmoOP = ImGuizmo::OPERATION::BOUNDS;
		return true;
	case gte::KeyCode::W:
		if (playing || !sGameEvents || gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right)) return false;
		sGuizmoOP = ImGuizmo::OPERATION::TRANSLATE;
		return true;
	case gte::KeyCode::E:
		if (playing || !sGameEvents || gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right)) return false;
		sGuizmoOP = ImGuizmo::OPERATION::ROTATE_Z;
		return true;
	case gte::KeyCode::R:
		if (playing || !sGameEvents || gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right)) return false;
		sGuizmoOP = ImGuizmo::OPERATION::SCALE;
		return true;
	case gte::KeyCode::N:
		if (playing) return false;
		if ( ctrl && shift)//Not implemented yet
			return false;
		else if (ctrl)
		{
			NewScene();
			return true;
		}
		return false;
	case gte::KeyCode::O:
		if (playing) return false;
		if (ctrl)//Not implemented yet
			return false;
		return false;
	case gte::KeyCode::S:
		if (playing) return false;
		if (ctrl && shift)
		{
			SaveSceneAs(gte::internal::CreateFileDialog(gte::internal::FileDialogType::Save, "Green Tea Scene (*.gtscene)\0*.gtscene\0"));
			return true;
		}
		else if (ctrl)
		{
			SaveScene();
			return true;
		}
		return false;
	default:
		return false;
	}
}

bool CupOfTea::OnScroll(float dx, float dy)
{
	if (!sGameEvents)
		return false;
	gte::Entity EditorCamera = gte::internal::GetContext()->ActiveScene->FindEntityWithUUID({});
	auto& ortho = EditorCamera.GetComponent<gte::OrthographicCameraComponent>();
	ortho.ZoomLevel -= dy * 0.25f;
	ortho.ZoomLevel = std::max(ortho.ZoomLevel, 0.25f);

	auto& cam = EditorCamera.GetComponent<gte::CameraComponent>();
	
	glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
	box *= glm::vec2(cam.AspectRatio, 1.0f);
	cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
	cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
	return true;
}

void CupOfTea::Start(void)
{
	gte::internal::GetContext()->DynamicLoader.Reload();
	gte::uuid id = mSceneHierarchyPanel.GetSelectedEntity().GetID();
	mSnapshot = gte::internal::GetContext()->ActiveScene;
	gte::internal::GetContext()->ActiveScene = gte::Scene::Copy(mSnapshot);
	gte::internal::GetContext()->Playing = true;
	gte::internal::GetContext()->ActiveScene->OnStart();
	if (id.IsValid())
	{
		gte::Entity selected = gte::internal::GetContext()->ActiveScene->FindEntityWithUUID(id);
		mSceneHierarchyPanel.SetSelectedEntity({ selected, gte::internal::GetContext()->ActiveScene });
	}

	const glm::vec2& viewportSize = gte::internal::GetContext()->ViewportSize;
	gte::internal::GetContext()->ActiveScene->OnViewportResize(static_cast<uint32>(viewportSize.x), static_cast<uint32>(viewportSize.y));
}

void CupOfTea::Stop(void)
{
	gte::uuid id = mSceneHierarchyPanel.GetSelectedEntity().GetID();
	gte::internal::GetContext()->ActiveScene->OnStop();
	delete gte::internal::GetContext()->ActiveScene;
	gte::internal::GetContext()->ActiveScene = gte::Scene::Copy(mSnapshot);
	if (id.IsValid())
	{
		gte::Entity selected = gte::internal::GetContext()->ActiveScene->FindEntityWithUUID(id);
		mSceneHierarchyPanel.SetSelectedEntity({ selected, gte::internal::GetContext()->ActiveScene });
	}
	delete mSnapshot;

	const glm::vec2& viewportSize = gte::internal::GetContext()->ViewportSize;
	gte::internal::GetContext()->ActiveScene->OnViewportResize(static_cast<uint32>(viewportSize.x), static_cast<uint32>(viewportSize.y));
	gte::internal::GetContext()->Playing = false;
}

#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

bool CupOfTea::DrawGuizmo(void)
{
	using namespace gte;
	if (gte::internal::GetContext()->Playing)//No guizmos while we play
		return false;

	Entity SelectedEntity = mSceneHierarchyPanel.GetSelectedEntity();
	if (!SelectedEntity)
		return false;
	if (!SelectedEntity.HasComponent<Transform2DComponent>())
		return false;

	Scene* ActiveScene = internal::GetContext()->ActiveScene;
	Entity EditorCamera = ActiveScene->FindEntityWithUUID({});
	const auto& ortho = EditorCamera.GetComponent<OrthographicCameraComponent>();
	const auto& cam = EditorCamera.GetComponent<CameraComponent>();
	const auto& camTC = EditorCamera.GetComponent<Transform2DComponent>();

	ImGuizmo::SetOrthographic(true);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

	glm::mat4 ViewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), { camTC.Position.x, camTC.Position.y, 1.0f }));
	glm::vec2 box = glm::vec2(ortho.VerticalBoundary * cam.AspectRatio, ortho.VerticalBoundary);
	box *= ortho.ZoomLevel;
	glm::mat4 ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, 0.0f, 2.0f);

	auto& tc = SelectedEntity.GetComponent<Transform2DComponent>();
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(tc.Rotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(tc.Scale.x, tc.Scale.y, 1.0f));

	//Check ansector's for transform and Manipulate accordigly
	const auto& rel = SelectedEntity.GetComponent<RelationshipComponent>();
	Entity Parent = { rel.Parent, ActiveScene };
	bool fpt = false;//Flag for weither we have found transform on progenitor
	if (rel.Parent != entt::null)
	{
		if (!Parent.HasComponent<TransformationComponent>())
		{
			auto parent = Parent.GetComponent<RelationshipComponent>().Parent;
			Parent = { parent, ActiveScene };
			while (parent != entt::null && !Parent.HasComponent<TransformationComponent>())//Search ansector with transform
			{
				parent = Parent.GetComponent<RelationshipComponent>().Parent;
				Parent = { parent, ActiveScene };
			}
			if (parent != entt::null)//Found ansector with transform
				fpt = true;
		}
		else
			fpt = true;
	}

	if (fpt)
	{
		const auto& PTransform = Parent.GetComponent<TransformationComponent>().Transform;
		transform = PTransform * transform;
		ImGuizmo::Manipulate(glm::value_ptr(ViewMatrix), glm::value_ptr(ProjectionMatrix),
			sGuizmoOP, ImGuizmo::MODE::LOCAL,
			glm::value_ptr(transform));
		transform = glm::inverse(PTransform) * transform;
	}
	else
	{
		ImGuizmo::Manipulate(glm::value_ptr(ViewMatrix), glm::value_ptr(ProjectionMatrix),
			sGuizmoOP, ImGuizmo::MODE::LOCAL,
			glm::value_ptr(transform));
	}

	if (ImGuizmo::IsUsing())
	{
		//SelectionFlag = false;
		glm::vec3 Position, Rotation, Scale;
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(Position), glm::value_ptr(Rotation), glm::value_ptr(Scale));
		tc.Position = Position;
		tc.Scale = glm::vec2(Scale.x, Scale.y);
		const float delta = Rotation.z - tc.Rotation;
		tc.Rotation += delta;
		ActiveScene->UpdateTransform(SelectedEntity);
		return true;
	}

	return false;
}

bool VisualizeOperation(void)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];

	ImGui::SetCursorPos({ 12.0f, 6.0f });
	ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.2f, 0.205f, 0.21f, 0.5f });
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
	ImGui::BeginChild("##actionpanel", { 104.0f, 24.0f }, false);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
	ImGui::PushFont(IconsFont);
	ImGui::SetCursorPosX(4.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
	if (sGuizmoOP == ImGuizmo::OPERATION::BOUNDS)
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.5, 0.0f, 1.0f });
	const bool bounds = ImGui::Button(ICON_FK_MOUSE_POINTER, { 24.0f, 24.0f });
	if (sGuizmoOP == ImGuizmo::OPERATION::BOUNDS)
		ImGui::PopStyleColor();
	ImGui::SameLine();
	if (sGuizmoOP == ImGuizmo::OPERATION::TRANSLATE)
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.5, 0.0f, 1.0f });
	const bool translate = ImGui::Button(ICON_FK_ARROWS, { 24.0f, 24.0f });
	if (sGuizmoOP == ImGuizmo::OPERATION::TRANSLATE)
		ImGui::PopStyleColor();
	ImGui::SameLine();
	if (sGuizmoOP == ImGuizmo::OPERATION::ROTATE_Z)
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.5, 0.0f, 1.0f });
	const bool rotate = ImGui::Button(ICON_FK_REFRESH, { 24.0f, 24.0f });
	if (sGuizmoOP == ImGuizmo::OPERATION::ROTATE_Z)
		ImGui::PopStyleColor();
	ImGui::SameLine();
	if (sGuizmoOP == (ImGuizmo::OPERATION::SCALE))
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.5, 0.0f, 1.0f });
	const bool scale = ImGui::Button(ICON_FK_EXPAND, { 24.0f, 24.0f });
	if (sGuizmoOP == (ImGuizmo::OPERATION::SCALE))
		ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::PopStyleVar(2);
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	if (bounds) sGuizmoOP = ImGuizmo::OPERATION::BOUNDS;
	else if (translate) sGuizmoOP = ImGuizmo::OPERATION::TRANSLATE;
	else if (rotate) sGuizmoOP = ImGuizmo::OPERATION::ROTATE_Z;
	else if (scale) sGuizmoOP = ImGuizmo::OPERATION::SCALE;
	if (bounds || translate || rotate || scale)
		return true;
	else
		return false;
}