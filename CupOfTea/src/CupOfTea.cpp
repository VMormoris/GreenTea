#include "CupOfTea.h"
#include "Panels/Panels.h"

#include <IconsFontAwesome4.h>

#include <ImGuizmo.h>

#include <gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

#include <gtc/matrix_transform.hpp>

#ifdef DEBUG_BUILD
static std::string binaries = "/bin/Debug-windows-x86_64/";
#else
static std::string binaries = "/bin/Release-windows-x86_64/";
#endif

using namespace GTE;

static ImGuizmo::OPERATION GuizmoOP;
static Entity SelectedEntity = { entt::null, nullptr };
static Entity EditorCameraEntity = { entt::null, nullptr };
static ProjectProperties pProps;
static entt::entity EnttNull = entt::null;

void RenderDockspace(void);

void CupOfTea::Update(float dt)
{
	//Check whether should we rotate the Editor's Camera
	if (m_GameEvents && !m_Playing)
	{
		auto& persp = EditorCameraEntity.GetComponent<PerspectiveCameraComponent>();
		auto& tc = EditorCameraEntity.GetComponent<TransformComponent>();

		glm::vec3 dir = glm::normalize(persp.Target - tc.Position);

		bool flag = false;
		if (Input::KeyPressed(KeyCode::UP) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::W)))
		{//Move Forward
			tc.Position += dir * m_CameraVelocity * dt;
			persp.Target += dir * m_CameraVelocity * dt;
			flag = true;
		}
		if (Input::KeyPressed(KeyCode::DOWN) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::S)))
		{//Move Backwars
			tc.Position -= dir * m_CameraVelocity * dt;
			persp.Target -= dir * m_CameraVelocity * dt;
			flag = true;
		}

		glm::vec3 right = glm::normalize(glm::cross(dir, persp.UpVector));

		if (Input::KeyPressed(KeyCode::RIGHT) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::D)))
		{//Move to the Right
			tc.Position += right * m_CameraVelocity * dt;
			persp.Target += right * m_CameraVelocity * dt;
			flag = true;
		}
		if (Input::KeyPressed(KeyCode::LEFT) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::A)))
		{//Move to the Left
			tc.Position -= right * m_CameraVelocity * dt;
			persp.Target -= right * m_CameraVelocity * dt;
			flag = true;
		}

		if (flag)
		{
			auto& transformation = EditorCameraEntity.GetComponent<TransformationComponent>();
			transformation = glm::translate(glm::mat4(1.0f), tc.Position)
				* glm::toMat4(glm::quat(glm::radians(tc.Rotation)));

			auto& cam = EditorCameraEntity.GetComponent<CameraComponent>();
			cam.ViewMatrix = glm::lookAt(tc.Position, persp.Target, persp.UpVector);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}

	}

	//Check if Viewport size change since the last frame
	if (GPU::FrameBufferSpecification spec = m_ViewportFBO->GetSpecification();
		(m_ViewportSize.x > 0.0f) && (m_ViewportSize.y > 0.0f) &&
		((spec.Width != (uint32)m_ViewportSize.x || spec.Height != m_ViewportSize.y)))
	{
		m_ViewportFBO->Resize((uint32)m_ViewportSize.x, (uint32)m_ViewportSize.y);
		GPU::FrameBufferSpecification newSpec = m_ViewportFBO->GetSpecification();
		RenderCommand::SetViewport(0, 0, newSpec.Width, newSpec.Height);
		m_ActiveScene->onViewportResize(newSpec.Width, newSpec.Height);
	}

	if (m_Playing) m_ActiveScene->Update(dt);

	//Render into the Viewport's Framebuffer
	m_ViewportFBO->Bind();
	m_ViewportFBO->Clear(1, &EnttNull);
	const auto& cam = &EditorCameraEntity.GetComponent<CameraComponent>();
	const glm::vec3 EyePos = EditorCameraEntity.GetComponent<TransformationComponent>().Transform[3];
	const glm::vec3 EyeDir = glm::normalize(EditorCameraEntity.GetComponent<PerspectiveCameraComponent>().Target-EyePos);
	m_ActiveScene->Render
	(
		m_Playing ? nullptr : cam,
		m_ViewportFBO,
		EyePos,
		EyeDir
	);
	m_ViewportFBO->Unbind();

	//Check if should Render at Camera's Framebuffer as well
	if (m_CamViewportSize.x > 0.0f && m_CamViewportSize.y > 0.0f && !m_Playing && SelectedEntity.Valid())
	{
		auto& cam = SelectedEntity.GetComponent<CameraComponent>();
		const auto& persp = SelectedEntity.GetComponent<PerspectiveCameraComponent>();
		cam.AspectRatio = m_CamViewportSize.x / m_CamViewportSize.y;

		cam.ProjectionMatrix = glm::perspective(glm::radians(persp.FoV), cam.AspectRatio, persp.Near, persp.Far);
		cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;

		m_CamFBO->Resize((uint32)m_CamViewportSize.x, (uint32)m_CamViewportSize.y);
		m_CamFBO->Bind();
		m_CamFBO->Clear(1, &EnttNull);
		RenderCommand::Clear();
		m_ActiveScene->Render(&cam, m_CamFBO, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 0.0f});
		m_CamFBO->Unbind();
	}

	//ImGui for Editor
	m_EditorLayer->Begin();
	ImGui::SetCurrentContext(m_EditorLayer->GetContext());
	ImGuizmo::BeginFrame();

	//Check whether should we rotate the Editor's Camera
	if (Input::MouseButtonPressed(MouseButtonType::Right) && m_GameEvents)
	{
		//Mouse Movement on screen
		const glm::vec2 LookAngleDest = glm::vec2{ -1.0f, -1.0f }  * 
			(glm::vec2{ ImGui::GetMousePos().x, ImGui::GetMousePos().y } - m_CursorPos);

		if (LookAngleDest != glm::vec2(0.0f))
		{//Need to rotate Editor's Camera
			const auto& tc = EditorCameraEntity.GetComponent<TransformComponent>();
			auto& persp = EditorCameraEntity.GetComponent<PerspectiveCameraComponent>();
			auto& cam = EditorCameraEntity.GetComponent<CameraComponent>();

			glm::vec3 dir = glm::normalize(persp.Target - tc.Position);
			glm::vec3 right = glm::normalize(glm::cross(dir, persp.UpVector));

			const float AngleSpeed = glm::pi<float>() * m_AngularSpeedFactor;
				
			glm::mat4 Rotation = glm::mat4(1.0f);
			Rotation *= glm::rotate(glm::mat4(1.0f), LookAngleDest.y * AngleSpeed, right);
			Rotation *= glm::rotate(glm::mat4(1.0f), LookAngleDest.x * AngleSpeed, persp.UpVector);

			dir = Rotation * glm::vec4(dir, 0.0f);
			float dist = glm::distance(tc.Position, persp.Target);
			persp.Target = tc.Position + dir * dist;
			cam.ViewMatrix = glm::lookAt(tc.Position, persp.Target, persp.UpVector);
			cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
		}
	}

	m_CursorPos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };//Update Mouse Position for next Frame

	RenderDockspace();

	//Render Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (DrawMenuItem(ICON_FA_FILE, "New", "Ctrl+N") && !m_Playing)
				NewScene();
			if (DrawMenuItem(ICON_FA_FOLDER_OPEN, "Open...", "Ctrl+O") && !m_Playing)
				OpenScene();
			if (DrawMenuItem(ICON_FA_FLOPPY_O, "Save As...", "Ctrl+Shift+S") && !m_Playing)
				SaveSceneAs();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Panels"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows, 
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);
			ImGui::MenuItem("Console Log", NULL, &m_Panels[0]);
			ImGui::MenuItem("Scene Manager", NULL, &m_Panels[1]);
			ImGui::MenuItem("Properties", NULL, &m_Panels[2]);
			ImGui::MenuItem("Viewport", NULL, &m_Panels[3]);
			ImGui::MenuItem("Scene Properties", NULL, &m_Panels[4]);

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (m_Panels[0])//Console Log Panel
	{
		if (ImGui::Begin("Console Log", &m_Panels[0]))
			GTE::RenderLogPanel();
		ImGui::End();
	}

	if (m_Panels[1])//Scene Manager Panel
	{
		if (ImGui::Begin("Scene Manager", &m_Panels[1]))
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[1];
			auto iconFont = io.Fonts->Fonts[0];
			const float width = 24.0f;
			const float offset = ImGui::GetContentRegionAvailWidth() - width + 4.0f;
			

			//Enviroment Entity
			const ImGuiTreeNodeFlags treeflags = (SelectedEntity == m_ActiveScene->GetSceneEntity() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
			ImGui::TreeNodeEx("Enviroment", treeflags);
			if (ImGui::IsItemClicked())
			{
				SelectedEntity = m_ActiveScene->GetSceneEntity();
				m_SceneManagerPanel.SetSelectedEntity(SelectedEntity);
			}
			ImGui::TreePop();


			//Globe Icon for Enviroment Enity
			ImGui::PushFont(iconFont);
			ImGui::SameLine();
			ImGui::SetCursorPos({ ImGui::GetCursorPosX() - 100.0f, ImGui::GetCursorPosY() + 2.0f });
			ImGui::Text(ICON_FA_GLOBE);//ICON_FA_CUBE//ICON_FA_CUBES//ICON_FA_LIGHTBULB_O
			ImGui::PopFont();
			ImGui::Separator();

			//Button for Creating new Entities
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 31.0f);
			ImGui::PushFont(boldFont);
			if (ImGui::Button("+", { width, 0.0f }))
				m_ActiveScene->CreateEntity();
			ImGui::PopFont();
			ImGui::Separator();

			//All other entities
			ImGui::BeginChild("Entities");
			m_SceneManagerPanel.Render();
			ImGui::EndChild();
			
			SelectedEntity = m_SceneManagerPanel.GetSelectedEntity();
			
		}
		ImGui::End();
	}

	//Rendering this Panel before Properties Panel
	//	Show the Properties Panel is showing by default
	if (m_Panels[4])//Scene Properties Panel
	{
		if (ImGui::Begin("Scene Properties"))
		{
			DrawComponent<CameraComponent>("Editor's Camera", EditorCameraEntity, [&](auto& cam) {
				UISettings settings;
				auto& tc = EditorCameraEntity.GetComponent<TransformComponent>();
				const bool PositionFlag = DrawVec3Control("Position", tc.Position, settings);
				auto& persp = EditorCameraEntity.GetComponent<PerspectiveCameraComponent>();
				const bool TargetFlag = DrawVec3Control("Target", persp.Target, settings);
				const bool UpVectorFlag = DrawVec3Control("Up Vector", persp.UpVector, settings);
				const bool FoVFlag = DrawFloatControl("FoV", persp.FoV, settings);
				const bool NearFlag = DrawFloatControl("Near Plane", persp.Near, settings);
				const bool FarFlag = DrawFloatControl("Far Plane", persp.Far, settings);
				DrawFloatControl("Velocity", m_CameraVelocity, settings);


				if (PositionFlag)
					EditorCameraEntity.UpdateMatrices();

				if (TargetFlag || UpVectorFlag)
				{
					cam.ViewMatrix = glm::lookAt(tc.Position, persp.Target, persp.UpVector);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
				}

				if (FoVFlag || NearFlag || FarFlag)
				{
					cam.ProjectionMatrix = glm::perspective(glm::radians(persp.FoV), cam.AspectRatio, persp.Near, persp.Far);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
				}
			});


			Entity sceneEntity = m_ActiveScene->GetSceneEntity();

			DrawComponent<EnviromentComponent>("Enviroment", sceneEntity, [](auto& env) {
				UISettings settings;
				if (DrawFilePicker("Map", env.SkyboxFilepath, ".png", settings))
					env.Skybox = AssetManager::RequestCubeMap(env.SkyboxFilepath.c_str());
			});

			DrawComponent<ScenePropertiesComponent>("Scene Properties", sceneEntity, [&](auto& sceneProp) {
				UISettings settings;
				settings.Clamp = glm::vec2(512.0f, FLT_MAX);
				settings.ResetValue = 1024.0f;
				settings.Speed = 1.0f;
				settings.ColumnWidth = 125.0f;
				if (DrawVec2Control("Shadowmap Res", sceneProp.ShadowmapResolution, settings))
					Renderer::ResizeShadowmapRes(sceneProp.ShadowmapResolution);
			});

			

		}
		ImGui::End();
	}

	if (m_Panels[2])//Properties Panel
	{
		if(ImGui::Begin("Properties", &m_Panels[2]))
			if(SelectedEntity.Valid()) RenderPropertiesPanel(SelectedEntity);
		ImGui::End();
	}

	if (m_Panels[3])//Viewport Panel
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		if (ImGui::Begin("Viewport", &m_Panels[3], ImGuiWindowFlags_NoTitleBar))
		{
			//Output the First Color attachement of our Framebuffer into the Viewport
			bool viewportHovered = ImGui::IsWindowHovered();
			bool viewportFocused = ImGui::IsWindowFocused();
			if (!viewportFocused || !viewportHovered) m_GameEvents = false;
			else m_GameEvents = true;
			ImVec2 size = ImGui::GetContentRegionAvail();
			ImVec2 windowSize = ImGui::GetWindowSize();
			m_ViewportSize = glm::vec2((float)size.x, (float)size.y);

			uint64 textID = m_ViewportFBO->GetColorAttachmentID(0);
			ImGui::Image((void*)textID, size, ImVec2(0, 1), ImVec2(1, 0));
			bool SelectionFlag = ImGui::IsItemClicked();

			GuizmoUpdate(SelectionFlag);

			//Check ther result of mouse picking to change the Selected Entity
			if (SelectionFlag)
			{
				auto [x, y] = ImGui::GetMousePos();
				x -= ImGui::GetWindowPos().x;
				y -= ImGui::GetWindowPos().y;
				y = windowSize.y - y;
				entt::entity enttID = entt::null;
				m_ViewportFBO->GetPixel(1, static_cast<int32>(x), static_cast<int32>(y), &enttID);
				if (enttID != entt::null)
				{
					SelectedEntity = { enttID, m_ActiveScene };
					m_SceneManagerPanel.SetSelectedEntity(SelectedEntity);
				}
			}

		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	//If Selected Entity has a Camera Component
	//	we creating a new Window to show it's Point of View
	if (SelectedEntity.Valid())
	{
		if (SelectedEntity.HasComponent<CameraComponent>() && !m_Playing)
		{
			ImGui::Begin("Camera's POV");
			ImVec2 size = ImGui::GetContentRegionAvail();
			m_CamViewportSize = glm::vec2((float)size.x, (float)size.y);
			uint64 textID = m_CamFBO->GetColorAttachmentID(0);
			ImGui::Image((void*)textID, size, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
		}
		else m_CamViewportSize = { 0.0f, 0.0f };
	}
	if (ImGui::Begin("Tools", 0, ImGuiWindowFlags_NoTitleBar))//Tools Panel
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::PushFont(io.Fonts->Fonts[0]);
		ImGui::Text(ICON_FA_PLAY);
		if (ImGui::IsItemClicked() && !m_Playing)
		{
			m_Playing = true;
			m_ActiveScene->TakeSnapshot();
			m_ActiveScene->onViewportResize(static_cast<uint32>(m_ViewportSize.x), static_cast<uint32>(m_ViewportSize.y));
			m_ActiveScene->LoadGameLogic();//Reload Game logic
		}
		ImGui::SameLine();
		ImGui::Text(ICON_FA_STOP);
		if (ImGui::IsItemClicked() && m_Playing)
		{
			m_Playing = false;
			m_ActiveScene->UnloadGameLogic();
			m_ActiveScene->ReloadSnapshot();
			m_ActiveScene->onViewportResize(static_cast<uint32>(m_ViewportSize.x), static_cast<uint32>(m_ViewportSize.y));
			EditorCameraEntity = m_ActiveScene->GetEditorCamera();
		}
		ImGui::PopFont();
	}
	ImGui::End();
	ImGui::ShowDemoWindow();
	ImGui::End();//Ending Dockspace
	m_EditorLayer->End();
}

CupOfTea::CupOfTea(const char* filepath) : Application("Cup Of Tea (GreenTea Editor)", -1, -1)
{
	AssetManager::RequestTexture("../Assets/Textures/Editor/Transparency.png");

	m_EditorLayer = ImGuiLayer::Create();
	m_EditorLayer->Init(m_Window->GetPlatformWindow(), m_Window->GetContext());

	//Initialize Framebuffers & Viewports
	GPU::FrameBufferSpecification spec;
	spec.Attachments = { GPU::TextureFormat::RGB8, GPU::TextureFormat::Int32, GPU::TextureFormat::Depth };
	spec.Width = m_Window->GetWidth();
	spec.Height = m_Window->GetHeight();
	
	m_ViewportFBO = GPU::FrameBuffer::Create(spec);
	m_CamFBO = GPU::FrameBuffer::Create(spec);

	m_ViewportSize = { (float)spec.Width, (float)spec.Height };
	m_CamViewportSize = { 0.0f, 0.0f };

	m_ActiveScene = new Scene();
	m_SceneManagerPanel.SetContext(m_ActiveScene);
	//Setup Editor's Camera
	EditorCameraEntity = m_ActiveScene->GetEditorCamera();


	//Read .gt file and load GameLogic and Last working Scene
	if (filepath)
	{
		//We currently assume is gt file
		//TODO: Add check for extension
		
		//Get Path & Project's name
		std::string Filepath(filepath);
		size_t pos = Filepath.find_last_of("/\\");
		pProps.ProjectPath = Filepath.substr(0, pos);
		size_t offset = pos + 1;
		pos = Filepath.find_last_of('.');
		pProps.ProjectName = Filepath.substr(offset, pos - offset);
		
		//Read File's content onto a string
		const char* content = utils::readfile(filepath);
		std::string project_str(content);
		delete content;
		
		//Acquire dll's Name
		pos = project_str.find("\r\n", 0);
		std::string dll = pProps.ProjectPath + binaries + pProps.ProjectName + "/" + project_str.substr(0, pos);
		GTE_TRACE_LOG("Loading dll: ", dll);
		m_ActiveScene->LoadGameLogic(dll.c_str());
		m_ActiveScene->UnloadGameLogic();//Unload so you can build project while the program is running

		//Check which was the last Active Scene (if there is one loaded)
		offset = pos + (project_str[pos]=='\r' ? 2 : 1);
		pProps.WorkingScene = offset ? project_str.substr(offset) : "";
		if (!pProps.WorkingScene.empty())
		{
			std::string scene_filepath = pProps.ProjectPath + "/Assets/Scenes/" + pProps.WorkingScene;
			m_ActiveScene->Load(scene_filepath.c_str());
		}
	}
}

CupOfTea::~CupOfTea(void)
{
	UNREGISTER(this);
	delete m_ActiveScene;
	delete m_CamFBO;
	delete m_ViewportFBO;
	delete m_EditorLayer;
}

void RenderDockspace(void)
{
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
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
}

void CupOfTea::GuizmoUpdate(bool& selectionFlag)
{
	const ImVec2 windowSize = ImGui::GetWindowSize();

	//Check whether any entity is Selected to render Guizmo
	if (SelectedEntity.Valid() && !m_Playing)
	{
		if (SelectedEntity.HasComponent<TransformComponent>())
		{
			const bool isEnv = SelectedEntity == m_ActiveScene->GetSceneEntity();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowSize.x, windowSize.y);

			const auto& persp = EditorCameraEntity.GetComponent<PerspectiveCameraComponent>();
			const auto& cam = EditorCameraEntity.GetComponent<CameraComponent>();
			const auto& camTC = EditorCameraEntity.GetComponent<TransformComponent>();
			auto& tc = SelectedEntity.GetComponent<TransformComponent>();

			glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(persp.FoV), windowSize.x / windowSize.y, persp.Near, persp.Far);
			glm::mat4 ViewMatrix = glm::lookAt(camTC.Position, persp.Target, persp.UpVector);

			glm::mat4 transform = glm::toMat4(glm::quat(glm::radians(tc.Rotation)));
			if (!isEnv)
			{
				transform = glm::translate(glm::mat4(1.0f), tc.Position) *
					transform *
					glm::scale(glm::mat4(1.0f), tc.Scale);
			}

			const auto& rel = SelectedEntity.GetComponent<RelationshipComponent>();

			bool manipulated;

			if (rel.Parent != entt::null)
			{
				Entity Parent = { rel.Parent, m_ActiveScene };
				glm::mat4 PTransform = Parent.GetComponent<TransformationComponent>().Transform;
				transform = PTransform * transform;

				manipulated = ImGuizmo::Manipulate
				(
					glm::value_ptr(ViewMatrix), glm::value_ptr(ProjectionMatrix),//Camera's Matrices
					GuizmoOP, ImGuizmo::MODE::LOCAL,//ImGuizmo properties 
					glm::value_ptr(transform)//Model Matrix
				);
				transform = glm::inverse(PTransform) * transform;
			}
			else
			{
				manipulated = ImGuizmo::Manipulate
				(
					glm::value_ptr(ViewMatrix), glm::value_ptr(ProjectionMatrix),//Camera's Matrices
					isEnv ? ImGuizmo::OPERATION::ROTATE : GuizmoOP, ImGuizmo::MODE::WORLD,//ImGuizmo properties
					glm::value_ptr(transform)//Model Matrix
				);
			}

			if (ImGuizmo::IsUsing())
			{
				selectionFlag = false;
				if (manipulated)
				{
					glm::vec3 Position, Rotation, Scale;
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(Position), glm::value_ptr(Rotation), glm::value_ptr(Scale));
					//Math::DecomposeTransform(transform, Position, Rotation, Scale);

					tc.Position = Position;
					tc.Scale = Scale;
					glm::vec3 delta = Rotation - tc.Rotation;
					tc.Rotation += delta;
					
					SelectedEntity.UpdateMatrices();
				}
			}
		}
	}
}

bool CupOfTea::onKeyDown(KeyCode keycode)
{
	switch (keycode)
	{
	case KeyCode::N:
		if (Input::KeyPressed(KeyCode::LEFT_CONTROL) || Input::KeyPressed(KeyCode::RIGHT_CONTROL))
		{
			NewScene();
			return true;
		}
		else
			return false;
	case KeyCode::O:
		if (Input::KeyPressed(KeyCode::LEFT_CONTROL) || Input::KeyPressed(KeyCode::RIGHT_CONTROL))
		{
			OpenScene();
			return true;
		}
		else
			return false;
	case KeyCode::S:
		if ((Input::KeyPressed(KeyCode::LEFT_CONTROL) || Input::KeyPressed(KeyCode::RIGHT_CONTROL)) &&
			(Input::KeyPressed(KeyCode::LEFT_SHIFT) || Input::KeyPressed(KeyCode::RIGHT_SHIFT)))
		{
			SaveSceneAs();
			return true;
		}
		else
			return false;
	case KeyCode::Q:
		if (!m_GameEvents) return false;
		GuizmoOP = ImGuizmo::OPERATION::BOUNDS;
		return true;
	case KeyCode::W:
		if (!m_GameEvents) return false;
		GuizmoOP = ImGuizmo::OPERATION::TRANSLATE;
		return true;
	case KeyCode::E:
		if (!m_GameEvents) return false;
		GuizmoOP = ImGuizmo::OPERATION::ROTATE;
		return true;
	case KeyCode::R:
		if (!m_GameEvents) return false;
		GuizmoOP = ImGuizmo::OPERATION::SCALE;
		return true;
	default:
		return false;
	}
}



void CupOfTea::NewScene(void)
{
	delete m_ActiveScene;
	m_ActiveScene = new Scene();
	m_ActiveScene->onViewportResize(static_cast<uint32>(m_ViewportSize.x), static_cast<uint32>(m_ViewportSize.y));
	EditorCameraEntity = m_ActiveScene->GetEditorCamera();

	m_SceneManagerPanel.SetContext(m_ActiveScene);
}

void CupOfTea::OpenScene(void)
{
	std::string filepath = CreateFileDialog(FileDialogType::Open, "Green Tea Scene (*.gtscene)\0*.gtscene\0");
	if (!filepath.empty())
	{
		m_ActiveScene->Load(filepath.c_str());
		m_ActiveScene->onViewportResize(static_cast<uint32>(m_ViewportSize.x), static_cast<uint32>(m_ViewportSize.y));
		EditorCameraEntity = m_ActiveScene->GetEditorCamera();
		std::string project_filepath = pProps.ProjectPath + "/" + pProps.ProjectName + ".gt";
		pProps.WorkingScene = filepath.substr(filepath.find_last_of("/\\"));
		std::string content = pProps.ProjectName + ".dll\n" + pProps.WorkingScene;
		utils::writefile(project_filepath.c_str(), content);

		//Update Scene properties
		Entity entity = m_ActiveScene->GetSceneEntity();
		auto& sceneProp = entity.GetComponent<ScenePropertiesComponent>();
		m_CameraVelocity = sceneProp.CamVelocity;

		Renderer::ResizeShadowmapRes(sceneProp.ShadowmapResolution);
	}
}

void CupOfTea::SaveSceneAs(void)
{
	std::string filepath = CreateFileDialog(FileDialogType::Save, "Green Tea Scene (*.gtscene)\0*.gtscene\0");
	if (!filepath.empty()) {
		if (filepath.find(".gtscene") == std::string::npos)
			filepath += ".gtscene";

		//Update Scene properties
		Entity entity = m_ActiveScene->GetSceneEntity();
		auto& sceneProp = entity.GetComponent<ScenePropertiesComponent>();
		sceneProp.CamVelocity = m_CameraVelocity;
		m_ActiveScene->Save(filepath.c_str());
	}
}