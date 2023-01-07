#include "CupOfTea.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <IconsForkAwesome.h>
#include <ImGuizmo.h>
#include "imspinner.h"

#include <gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

#include <fstream>
#include <Engine/Renderer/Renderer2D.h>

static gte::GPU::Texture* sIcon = nullptr;
static bool sLoaded = false;
static ImGuizmo::OPERATION sGuizmoOP = ImGuizmo::OPERATION::BOUNDS;
static gte::GPU::FrameBuffer* sCamFBO = nullptr;
static glm::vec2 sCamViewport;
static constexpr entt::entity EnttNull = entt::null;
static bool sGameEvents = true;
static std::atomic<int> sExportIndex = -1;

static bool VisualizeOperation(void);
static void DrawExportPopup(void);
static void DrawExportProgress(void);
static void ExportGame(const std::filesystem::path& location, const std::filesystem::path& icon, const std::filesystem::path& logo);

void CupOfTea::Update(float dt)
{
	constexpr float ClearColor = 30.0f / 255.0f;
	gte::Scene* scene = gte::internal::GetContext()->ActiveScene;
	gte::GPU::FrameBuffer* viewportFBO = gte::internal::GetContext()->ViewportFBO;

	gte::Entity EditorCamera = scene->FindEntityWithUUID({});
	if (sGameEvents && !gte::internal::GetContext()->Playing)
	{
		auto& tc = EditorCamera.GetComponent<gte::TransformComponent>();
		auto& presp = EditorCamera.GetComponent<gte::PerspectiveCameraComponent>();
		const auto& settings = EditorCamera.GetComponent<gte::Settings>();
		
		glm::vec3 dir = glm::normalize(presp.Target - tc.Position);
		glm::vec3 fraction = dir * settings.CameraVelocity * dt;

		glm::vec3 offset = { 0.0f, 0.0f, 0.0f };
		//Front & back
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::W))
			offset = fraction;
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::S))
			offset -= fraction;
		//Right & Left
		const glm::vec3 right = glm::normalize(glm::cross(dir, presp.UpVector));
		fraction = right * settings.CameraVelocity * dt;
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::D))
			offset += fraction;
		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Right) && gte::Input::IsKeyPressed(gte::KeyCode::A))
			offset -= fraction;

		if (gte::Input::IsMouseButtonPressed(gte::MouseButtonType::Wheel))
		{
			constexpr float speed = glm::pi<float>() * 0.002;

			uint32 dx, dy;
			gte::Input::GetMouseOffset(dx, dy);
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), dy * speed, right);
			rotation *= glm::rotate(glm::mat4(1.0f), dx * speed, presp.UpVector);

			dir = rotation * glm::vec4(dir, 0.0f);
			presp.Target = tc.Position + dir * glm::distance(tc.Position, presp.Target);
		}

		tc.Position += offset;
		presp.Target += offset;
		scene->UpdateTransform(EditorCamera);
	}

    Application::Update(dt);
    gte::Window* window = gte::internal::GetContext()->GlobalWindow;
	glm::vec2& viewportSize = gte::internal::GetContext()->ViewportSize;
	if (gte::GPU::FrameBufferSpecification spec = viewportFBO->GetSpecification();
		(viewportSize.x > 0.0f) && (viewportSize.y > 0.0f) &&
		((spec.Width != viewportSize.x || spec.Height != viewportSize.y)))
	{
		viewportFBO->Resize((uint32)viewportSize.x, (uint32)viewportSize.y);
		gte::GPU::FrameBufferSpecification newSpec = viewportFBO->GetSpecification();
		gte::internal::GetContext()->PixelBufferObject->Resize(newSpec.Width, newSpec.Height);
		scene->OnViewportResize(newSpec.Width, newSpec.Height);
	}

	if (gte::internal::GetContext()->AssetWatcher.Reload())
	{
		const auto& changes = gte::internal::GetContext()->AssetWatcher.GetChanges();
		for (const auto& id : changes)
			gte::internal::GetContext()->AssetManager.RemoveAsset(id);
		sLoaded = false;
	}
	if (gte::internal::GetContext()->AssetWatcher.IsBuilding() && gte::internal::GetContext()->Playing)
	{
		Stop();
		return;
	}

	if (gte::internal::GetContext()->Playing)
		scene->Update(dt);

	const auto spec = viewportFBO->GetSpecification();
	gte::RenderCommand::SetViewport(0, 0, spec.Width, spec.Height);
    gte::RenderCommand::SetClearColor({ ClearColor, ClearColor, ClearColor, 1.0f });
	gte::Renderer2D::BeginFrame(viewportFBO);
	if (gte::internal::GetContext()->Playing)
	{
		if (gte::Entity camera = scene->GetPrimaryCameraEntity())
			scene->Render(camera.GetComponent<gte::CameraComponent>());
	}
	else
		scene->Render(EditorCamera.GetComponent<gte::CameraComponent>());
	OnOverlayRenderer();

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
		gte::RenderCommand::SetViewport(0, 0, static_cast<uint32>(sCamViewport.x), static_cast<uint32>(sCamViewport.y));
		gte::RenderCommand::SetClearColor({ ClearColor, ClearColor, ClearColor, 1.0f });
		gte::Renderer2D::BeginFrame(sCamFBO, false);
		scene->Render(cam.EyeMatrix, sCamFBO);
	}

	mAnimationPanel.Update(dt);
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
		if (gte::Entity camera = scene->GetPrimaryCameraEntity())
			eyeMatrix = camera.GetComponent<gte::CameraComponent>();
		else
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

	bool exportgame = false;
	if (ImGui::BeginMainMenuBar())
	{
		//ImGui::Image(sIcon->GetID(), { 64.0f, 51.0f }, { 0, 1 }, { 1, 0 });
		if (ImGui::BeginMenu("File"))
		{
			const char biggest[] = "Save Scene As...Ctrl+Shift+N";
			if (gte::gui::DrawMenuItem(ICON_FK_CUBES, "New Scene", "Ctrl+N", biggest))
				NewScene();
			//if (gte::gui::DrawMenuItem(ICON_FK_BOOK, "New Project", "Ctrl+Shift+N", biggest))
			//{
			//}
			//if (gte::gui::DrawMenuItem(ICON_FK_FOLDER_OPEN, "Open Project...", "Ctrl+O", biggest))
			//{
			//	
			//}
			if (gte::gui::DrawMenuItem(ICON_FK_FLOPPY_O, "Save Scene", "Ctrl+S", biggest))
				SaveScene();
			if (gte::gui::DrawMenuItem(ICON_FK_FILE, "Save Scene As...", "Ctrl+Shift+S", biggest))
				SaveSceneAs(gte::internal::CreateFileDialog(gte::internal::FileDialogType::Save, "Green Tea Scene (*.gtscene)\0*.gtscene\0"));
			ImGui::Separator();
			if (gte::gui::DrawMenuItem(ICON_FK_GAMEPAD, "Export Game", nullptr, biggest))
				exportgame = true;
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
	if (exportgame)
		ImGui::OpenPopup("Export Game##Popup");
	DrawExportPopup();
	DrawExportProgress();
	ImGui::End();//End Dockspace
	
	if (mPanels[0] && !mPanels[7])
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		if (ImGui::Begin("Viewport", &mPanels[0]))
		{
			sGameEvents = ImGui::IsWindowFocused();
			ImVec2 windowPos = ImGui::GetWindowPos();
			gte::internal::GetContext()->ViewportOffset = { windowPos.x, windowPos.y };
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 size = ImGui::GetContentRegionAvail();
			gte::internal::GetContext()->ViewportSize = { size.x, size.y };
			uint64 textID = gte::internal::GetContext()->ViewportFBO->GetColorAttachmentID(0);
			ImGui::Image((void*)textID, size, { 0, 1 }, { 1, 0 });
			bool selection = ImGui::IsMouseReleased(ImGuiPopupFlags_MouseButtonLeft) && sGuizmoOP == ImGuizmo::OPERATION::BOUNDS && sGameEvents;

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
				gte::Entity entity = gte::Input::GetHoveredEntity();
				if (entity)
					mSceneHierarchyPanel.SetSelectedEntity(entity);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		{//Tools Panel
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
			ImGui::Begin("##Tools", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
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
			ImGui::End();
			ImGui::PopStyleVar(2);
		}
	}

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

	if (mPanels[3] && !mPanels[7])
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

	gte::uuid animation;
	if (mPanels[1])
	{
		if (ImGui::Begin("Content Broswer", &mPanels[1], ImGuiWindowFlags_NoCollapse))
		{
			mBrowserPanel.Draw();
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && gte::Input::IsKeyPressed(gte::KeyCode::DEL))
				mBrowserPanel.DeleteSelected();
			animation = mBrowserPanel.GetAnimation();
		}
		ImGui::End();
	}

	if (mPanels[5])
	{
		if (ImGui::Begin("About##popup", &mPanels[5], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize))
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
			ImGui::BulletText("msdf-atlas-gen");
			ImGui::BulletText("openal-soft");
			ImGui::BulletText("premake5");
			ImGui::BulletText("stb");
			ImGui::BulletText("yaml-cpp");

			ImGui::PushFont(BoldFont);
			ImGui::Text("\nAssets");
			ImGui::PopFont();
			ImGui::BulletText("Fork Awesome");
			ImGui::BulletText("Roboto");
			ImGui::BulletText("Papirus icon theme");

			ImGui::PushFont(BoldFont);
			ImGui::Text("\nContributors");
			ImGui::PopFont();
			ImGui::BulletText("Manoussos Manouras");

			ImGui::PushFont(BoldFont);
			ImGui::Text("\nQA Testers");
			ImGui::PopFont();
			ImGui::BulletText("Manoussos Manouras");
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
				auto& tc = SceneEntity.GetComponent<gte::TransformComponent>();
				if (gte::gui::DrawVec3Control("Position", tc.Position, settings))
				{
					auto& transorm = glm::translate(glm::mat4(1.0f), tc.Position);
					cam.ViewMatrix = glm::inverse(transorm);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
				}

				auto& Velocity = SceneEntity.GetComponent<gte::Settings>().CameraVelocity;
				settings.MinFloat = 0.25f;
				settings.MaxFloat = FLT_MAX;
				gte::gui::DrawFloatControl("Velocity", Velocity, settings);

				if (SceneEntity.HasComponent<gte::OrthographicCameraComponent>())//PerspectiveCameraComponent
				{
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
				}
				if (SceneEntity.HasComponent<gte::PerspectiveCameraComponent>())
				{
					auto& persp = SceneEntity.GetComponent<gte::PerspectiveCameraComponent>();
					bool changed = gte::gui::DrawVec3Control("Target", persp.Target, settings, "Position in the world that the camera is looking at");
					settings.MaxFloat = 180.0f;
					settings.MinFloat = 30.0f;
					settings.Speed = 0.25f;
					if (gte::gui::DrawFloatControl("Field of View", persp.FoV, settings, "Camera's field of view"))
						changed = true;
					settings.MinFloat = 0.1f;
					settings.MaxFloat = FLT_MAX;
					settings.Speed = 0.1f;
					if (gte::gui::DrawFloatControl("Near", persp.Near, settings, "Camera's near plane"))
						changed = true;
					settings.MinFloat = persp.Near + 0.1f;
					settings.MaxFloat = FLT_MAX;
					settings.Speed = 0.1f;
					if (gte::gui::DrawFloatControl("Far", persp.Far, settings, "Camera's far plane"))
						changed = true;

					if (changed)
					{
						cam.ViewMatrix = glm::lookAt(tc.Position, persp.Target, persp.UpVector);
						cam.ProjectionMatrix = glm::perspective(persp.FoV, cam.AspectRatio, persp.Near, persp.Far);
						cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
					}
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

	if (mPanels[7])
	{
		mAnimationPanel.Draw(mPanels[7]);
	}

	//ImGui::ShowDemoWindow();
    gui->End();
	if (animation.IsValid())
	{
		mPanels[7] = true;
		mAnimationPanel.SetAnimation(animation);
	}
}

CupOfTea::CupOfTea(const std::string& filepath)
	: Application({"CupOfTea (Green Tea editor)", 0, 0, 1080, 720, false, true, true}), mBrowserPanel("Assets")
{
    gte::Window* window = gte::internal::GetContext()->GlobalWindow;
    gui = gte::gui::ImGuiLayer::Create();
    gui->Init(window->GetPlatformWindow(), window->GetContext());

	gte::Image img("../Assets/Icons/Logo.png");
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
	if (std::filesystem::exists("Assets/" + scenename) && !scenename.empty())
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
	spec.Attachments = { gte::GPU::TextureFormat::RGB8 };
	spec.Width = window->GetWidth();
	spec.Height = window->GetHeight();
	gte::internal::GetContext()->ViewportFBO = gte::GPU::FrameBuffer::Create(spec);

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
	gte::uuid id;
	if (gte::Entity selectedEntity = mSceneHierarchyPanel.GetSelectedEntity())
		id = selectedEntity.GetID();
	mSnapshot = gte::internal::GetContext()->ActiveScene;
	gte::internal::GetContext()->Playing = true;
	gte::internal::GetContext()->ActiveScene = gte::Scene::Copy(mSnapshot);
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
	gte::uuid id;
	if (gte::Entity selectedEntity = mSceneHierarchyPanel.GetSelectedEntity())
		id = selectedEntity.GetID();
	
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
	if (!SelectedEntity.HasComponent<TransformComponent>())
		return false;

	Scene* ActiveScene = internal::GetContext()->ActiveScene;
	Entity EditorCamera = ActiveScene->FindEntityWithUUID({});
	const auto& persp = EditorCamera.GetComponent<PerspectiveCameraComponent>();
	const auto& cam = EditorCamera.GetComponent<CameraComponent>();
	const auto& camTC = EditorCamera.GetComponent<TransformComponent>();

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

	glm::mat4 ViewMatrix = glm::lookAt(camTC.Position, persp.Target, persp.UpVector);
	glm::mat4 ProjectionMatrix = glm::perspective(persp.FoV, cam.AspectRatio, persp.Near, persp.Far);

	auto& tc = SelectedEntity.GetComponent<TransformComponent>();
	glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(tc.Rotation)));
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Position) *
		rotation *
		glm::scale(glm::mat4(1.0f), tc.Scale);

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
		tc.Scale = Scale;
		const glm::vec3 delta = Rotation - tc.Rotation;
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

void DrawExportPopup(void)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];

	static std::string error = "";
	bool exportprogress = false;
	if (ImGui::BeginPopupModal("Export Game##Popup", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		static std::string location = "";
		static std::string icon = "";
		static std::string logo = "";

		//Location input
		ImGui::Text("Location:");
		ImGui::SameLine();
		char* buffer = new char[location.size() + 12];
		memcpy(buffer, location.c_str(), location.size() + 1);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		if (ImGui::InputText("##location", buffer, location.size() + 12))
			location = std::string(buffer);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		ImGui::PushFont(IconsFont);
		if (ImGui::Button(ICON_FK_FOLDER, { 24.0f, 24.0f }))
		{
			std::string temp = gte::internal::PeekDirectory();
			if (!temp.empty())
			{
				location = temp;
				std::replace(location.begin(), location.end(), '\\', '/');
			}

		}
		ImGui::PopFont();
		ImGui::SameLine();
		const float y = ImGui::GetCursorPosY();
		ImGui::SetCursorPos({ ImGui::GetCursorPosX() - 6.0f, y - 6.0f });
		ImGui::Text("*");
		ImGui::SameLine();
		ImGui::SetCursorPosY(y);
		ImGui::Dummy({ 0.0f, 0.0f });
		delete[] buffer;


		//Icon input
		ImGui::PushID("icon");
		ImGui::Text("Icon:");
		ImGui::SameLine(0.0f, 38.0f);
		buffer = new char[icon.size() + 12];
		memcpy(buffer, icon.c_str(), icon.size() + 1);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		if (ImGui::InputText("##icon", buffer, icon.size() + 12))
			icon = std::string(buffer);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		ImGui::PushFont(IconsFont);
		if (ImGui::Button(ICON_FK_FILE_IMAGE_O, { 24.0f, 24.0f }))
		{
			std::string temp = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "Icons (*.ico)\0*.ico\0");
			if (!temp.empty())
			{
				icon = temp;
				std::replace(icon.begin(), icon.end(), '\\', '/');
			}
		}
		ImGui::PopFont();
		ImGui::PopID();

		//Logo
		ImGui::PushID("logo");
		ImGui::Text("Logo:");
		ImGui::SameLine(0.0f, 33.0f);
		buffer = new char[logo.size() + 12];
		memcpy(buffer, logo.c_str(), logo.size() + 1);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		if (ImGui::InputText("##logo", buffer, logo.size() + 12))
			logo = std::string(buffer);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		ImGui::PushFont(IconsFont);
		if (ImGui::Button(ICON_FK_FILE_IMAGE_O, { 24.0f, 24.0f }))
		{
			std::string temp = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "All Images(*.png, *jpg, *.jpeg)\0*.png;*.jpg;*.jpeg\0PNG file (*.png)\0*.png\0JPG file(*.jpg, *.jpeg)\0*.jpg; *.jpeg\0");
			if (!temp.empty())
			{
				logo = temp;
				std::replace(logo.begin(), logo.end(), '\\', '/');
			}
		}
		ImGui::PopFont();
		ImGui::PopID();

		//Buttons
		ImGui::Dummy({ 0.0f, 0.0f });
		ImGui::SameLine(0.0f, 193.0f);
		if (ImGui::Button("Export"))
		{

			if (location.empty())
			{
				error = "Location field must be filled.";
				ImVec2 size = ImGui::CalcTextSize(error.c_str());
				ImGui::SetNextWindowSize({ size.x + 16.0f, 0.0f });
				ImGui::OpenPopup("Error##popup");
			}
			else if (!std::filesystem::is_directory(location))
			{
				error = "Location must describe a valid path to a directory.";
				ImVec2 size = ImGui::CalcTextSize(error.c_str());
				ImGui::SetNextWindowSize({ size.x + 16.0f, 0.0f });
				ImGui::OpenPopup("Error##popup");
			}
			else
			{
				ExportGame(location, icon, logo);
				location = icon = logo = "";
				ImGui::CloseCurrentPopup();
				exportprogress = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		if (ImGui::BeginPopupModal("Error##popup", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text(error.c_str());
			ImGui::Dummy({ 0.0f, 0.0f });
			ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 80.0f);
			if (ImGui::Button("Ok", { 80.0f, 0.0f }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	if (exportprogress)
		ImGui::OpenPopup("Export Progress##Popup");
}

void DrawExportProgress(void)
{
	static float accumulator = 0.0f;
	if (sExportIndex < 0)
	{
		accumulator = 0.0f;
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	accumulator += io.DeltaTime;

	static constexpr char* strings[] = {
		"Building Stand Alone App",
		"Building project",
		"Combining everything into a folder"
	};
	
	if (ImGui::BeginPopupModal("Export Progress##Popup", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		const float strWidth = ImGui::CalcTextSize(strings[sExportIndex]).x;
		ImGui::Text(strings[sExportIndex]);
		ImGui::SameLine();
		int index = (int)accumulator % 4;
		switch (index)
		{
		case 0: ImGui::Text("   "); break;
		case 1: ImGui::Text(".  "); break;
		case 2: ImGui::Text(".. "); break;
		case 3: ImGui::Text("..."); break;
		}
		ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - strWidth - 32.0f);
		ImSpinner::SpinnerAngTriple("MySpinner", 3.0f, 6.0f, 9.0f, 2.0f, 0xffffffff, 0xff0000ff, 0xffffffff);
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
}

void ExportGame(const std::filesystem::path& location, const std::filesystem::path& icon, const std::filesystem::path& logo)
{
	sExportIndex = 0;
	std::thread handle = std::thread([=]() {
		auto GreenTeaDir = gte::internal::GetContext()->GreenTeaDir;
		auto prjname = std::filesystem::current_path().filename().string();

		//Create StandAlone solution in case it doesn't exist
		std::string command = "premake5 --file=" + GreenTeaDir + "/StandAlone/premake5.lua vs2019";
		std::system(command.c_str());

		//Build StandAlone app in case it hasn't been build yet
		std::ofstream os(GreenTeaDir + "/StandAlone/StandAlone.rc");
		if (!icon.empty())
			os << "logo ICON \"" + icon.string() + "\"";
		else
			os << "logo ICON \"../Assets/Icons/GreenTea.ico\"";
		os.close();
		std::filesystem::remove(GreenTeaDir + "/StandAlone/StandAlone.aps");
		std::filesystem::remove(GreenTeaDir + "/StandAlone/bin-int/Dist-windows/StandAlone/StandAlone.res");
		command = "devenv " + GreenTeaDir + "/StandAlone/StandAlone.sln -Build Dist";
		std::system(command.c_str());

		//Build project for StandAlone
		sExportIndex++;
		std::system("premake5 vs2019");
		command = "devenv " + prjname + ".sln -Build StandAlone";
		std::system(command.c_str());

		//Create folder that we hold the standalone version
		sExportIndex++;
		auto binaries = location / (prjname + "/bin");
		std::filesystem::create_directories(binaries);
		std::filesystem::copy(GreenTeaDir + "/StandAlone/bin/Dist-windows/GreenTea/GreenTea.dll", binaries, std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy(GreenTeaDir + "/StandAlone/bin/Dist-windows/StandAlone/StandAlone.exe", binaries, std::filesystem::copy_options::overwrite_existing);

		auto gameData = location / (prjname + "/GameData");
		std::filesystem::create_directories(gameData / ".gt");
		std::filesystem::copy(std::filesystem::current_path() / ("bin/StandAlone-windows/" + prjname + "/" + prjname + ".dll"), gameData / ".gt", std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy(std::filesystem::current_path() / "Assets", gameData / "Assets", std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy(std::filesystem::current_path() / (prjname + ".gt"), gameData, std::filesystem::copy_options::overwrite_existing);

		std::filesystem::create_directories(location / (prjname + "/Assets/Icons"));
		std::filesystem::copy(GreenTeaDir + "/Assets/Icons/GreenTeaLogo.png", location / (prjname + "/Assets/Icons/GreenTeaLogo.png"), std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy(GreenTeaDir + "/Assets/Shaders", location / (prjname + "/Assets/Shaders"), std::filesystem::copy_options::overwrite_existing);
		if (logo.empty())
			std::filesystem::copy(GreenTeaDir + "/Assets/Icons/Logo.png", location / (prjname + "/Assets/Icons/Logo.png"), std::filesystem::copy_options::overwrite_existing);
		else
			std::filesystem::copy(logo, location / (prjname + "/Assets/Icons/Logo.png"), std::filesystem::copy_options::overwrite_existing);
		sExportIndex = -1;
	});
	handle.detach();
}