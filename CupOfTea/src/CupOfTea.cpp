#include "CupOfTea.h"
#include "Panels/Panels.h"

#include <IconsFontAwesome4.h>

#include <ImGuizmo.h>

#include <gtc/type_ptr.hpp>
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
	if (m_GameEvents && !m_Playing)
	{
		const auto& ortho = EditorCameraEntity.GetComponent<OrthographicCameraComponent>();
		auto& transform = EditorCameraEntity.GetComponent<Transform2DComponent>();

		bool flag = false;
		if (Input::KeyPressed(KeyCode::UP) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::W)))
		{
			transform.Position.y += m_CameraVelocity.y * dt;
			flag = true;
		}
		if (Input::KeyPressed(KeyCode::DOWN) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::S)))
		{
			transform.Position.y -= m_CameraVelocity.y * dt;
			flag = true;
		}
		if (Input::KeyPressed(KeyCode::RIGHT) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::D)))
		{
			transform.Position.x += m_CameraVelocity.x * dt;
			flag = true;
		}
		if (Input::KeyPressed(KeyCode::LEFT) || (Input::MouseButtonPressed(MouseButtonType::Right) && Input::KeyPressed(KeyCode::A)))
		{
			transform.Position.x -= m_CameraVelocity.x * dt;
			flag = true;
		}
		if (flag)
		{
			auto& transformation = EditorCameraEntity.GetComponent<TransformationComponent>();
			transformation = glm::translate(glm::mat4(1.0f), transform.Position)
				* glm::rotate(glm::mat4(1.0f), glm::radians(transform.Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

			auto& cam = EditorCameraEntity.GetComponent<CameraComponent>();
			cam.ViewMatrix = glm::inverse(transformation.Transform);
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
	RenderCommand::SetClearColor({ 1.0f, 0.0f, 1.0f, 1.0f });
	RenderCommand::Clear();
	m_ViewportFBO->Clear(1, &EnttNull);
	m_ActiveScene->Render(m_Playing ? nullptr : &EditorCameraEntity.GetComponent<CameraComponent>().EyeMatrix);
	m_ViewportFBO->Unbind();

	//Check if should Render at Camera's Framebuffer as well
	if (m_CamViewportSize.x > 0.0f && m_CamViewportSize.y > 0.0f && !m_Playing && SelectedEntity.Valid())
	{
		auto& cam = SelectedEntity.GetComponent<CameraComponent>();
		const auto& ortho = SelectedEntity.GetComponent<OrthographicCameraComponent>();
		cam.AspectRatio = m_CamViewportSize.x / m_CamViewportSize.y;

		glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
		box *= glm::vec2(cam.AspectRatio, 1.0f);

		cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
		cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;

		m_CamFBO->Resize((uint32)m_CamViewportSize.x, (uint32)m_CamViewportSize.y);
		m_CamFBO->Bind();
		RenderCommand::SetViewport(0, 0, (uint32)m_CamViewportSize.x, (uint32)m_CamViewportSize.y);
		RenderCommand::SetClearColor({ 1.0f, 0.0f, 1.0f, 1.0f });
		RenderCommand::Clear();
		m_ActiveScene->Render(&cam.EyeMatrix);
		m_CamFBO->Unbind();
		RenderCommand::SetViewport(0, 0, (uint32)m_ViewportSize.x, (uint32)m_ViewportSize.y);
	}

	//ImGui for Editor
	m_EditorLayer->Begin();
	ImGui::SetCurrentContext(m_EditorLayer->GetContext());
	ImGuizmo::BeginFrame();

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
			if (ImGui::Button("Add Entity")) m_ActiveScene->CreateEntity();			
			ImGui::Separator();
			m_SceneManagerPanel.Render();
			SelectedEntity = m_SceneManagerPanel.GetSelectedEntity();
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
		if (ImGui::Begin("Viewport", &m_Panels[3]))
		{
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


			if (SelectedEntity.Valid() && !m_Playing)
			{
				if (SelectedEntity.HasComponent<Transform2DComponent>())
				{
					ImGuizmo::SetOrthographic(true);
					ImGuizmo::SetDrawlist();

					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowSize.x, windowSize.y);

					const auto& ortho = EditorCameraEntity.GetComponent<OrthographicCameraComponent>();
					const auto& cam = EditorCameraEntity.GetComponent<CameraComponent>();
					const auto& camTC = EditorCameraEntity.GetComponent<Transform2DComponent>();

					auto& tc = SelectedEntity.GetComponent<Transform2DComponent>();

					glm::mat4 ViewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(camTC.Position.x, camTC.Position.y, 1.0f)));

					glm::vec2 box = glm::vec2(ortho.VerticalBoundary * cam.AspectRatio, ortho.VerticalBoundary);
					box *= ortho.ZoomLevel;
					glm::mat4 ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, 0.0f, 2.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Position) *
						glm::rotate(glm::mat4(1.0f), glm::radians(tc.Rotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
						glm::scale(glm::mat4(1.0f), glm::vec3(tc.Scale.x, tc.Scale.y, 1.0f));

					const auto& rel = SelectedEntity.GetComponent<RelationshipComponent>();

					if (rel.Parent != entt::null)
					{
						Entity Parent = { rel.Parent, m_ActiveScene };
						glm::mat4 PTransform = Parent.GetComponent<TransformationComponent>().Transform;
						transform = PTransform * transform;
						ImGuizmo::Manipulate(glm::value_ptr(ViewMatrix), glm::value_ptr(ProjectionMatrix),
							GuizmoOP, ImGuizmo::MODE::LOCAL,
							glm::value_ptr(transform));
						transform = glm::inverse(PTransform) * transform;
					}
					else
					{
						ImGuizmo::Manipulate(glm::value_ptr(ViewMatrix), glm::value_ptr(ProjectionMatrix),
							GuizmoOP, ImGuizmo::MODE::LOCAL,
							glm::value_ptr(transform));
					}

					if (ImGuizmo::IsUsing())
					{
						SelectionFlag = false;
						glm::vec3 Position, Rotation, Scale;
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(Position), glm::value_ptr(Rotation), glm::value_ptr(Scale));
						tc.Position = Position;
						tc.Scale = glm::vec2(Scale.x, Scale.y);
						const float delta = Rotation.z - tc.Rotation;
						tc.Rotation += delta;
						SelectedEntity.UpdateMatrices();
					}
				}
			}
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

	if (m_Panels[4])//Scene Properties Panel
	{
		if (ImGui::Begin("Scene Properties"))
		{
			DrawComponent<ScenePropertiesComponent>("Physics Engine", m_ActiveScene->GetSceneEntity(), [](auto& sceneProp) {
				UISettings settings;
				settings.ColumnWidth = 125.f;
				DrawVec2Control("Gravity", sceneProp.Gravity, settings);
				settings.Speed = 5.0f;
				settings.Clamp.x = 30;
				settings.Clamp.y = FLT_MAX;
				DrawIntControl("Tick Rate", sceneProp.Rate, settings);
				settings.Speed = 1.0f;
				settings.Clamp.x = 1;
				DrawIntControl("Velocity Iterations", sceneProp.VelocityIterations, settings);
				DrawIntControl("Position Iterations", sceneProp.PositionIterations, settings);
			});

			DrawComponent<CameraComponent>("Editor's Camera", EditorCameraEntity, [&](auto& cam) {
				UISettings settings;
				settings.ColumnWidth = 125.0f;
				auto& transform = EditorCameraEntity.GetComponent<Transform2DComponent>();
				glm::vec2 Position = glm::vec2{ transform.Position.x, transform.Position.y };

				const bool PositionFlag = DrawVec2Control("Position", Position, settings);
				DrawVec2Control("Velocity", m_CameraVelocity, settings);
				auto& ortho = EditorCameraEntity.GetComponent<OrthographicCameraComponent>();
				const bool ZoomFlag = DrawFloatControl("Zoom Level", ortho.ZoomLevel, settings);
				const bool BoundaryFlag = DrawFloatControl("Vetical Boundary", ortho.VerticalBoundary, settings);
				if (PositionFlag)
				{
					transform.Position.x = Position.x;
					transform.Position.y = Position.y;
					EditorCameraEntity.UpdateMatrices();
				}
				if (ZoomFlag || BoundaryFlag)
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

	if (ImGui::Begin("Tools"))
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

	ImGui::End();//Ending Dockspace
	m_EditorLayer->End();
}

CupOfTea::CupOfTea(const char* filepath) : Application("Cup Of Tea (GreenTea Editor)", -1, -1)
{
	REGISTER(EventType::MouseScroll, this, &CupOfTea::onScroll)

	m_EditorLayer = ImGuiLayer::Create();
	m_EditorLayer->Init(m_Window->GetPlatformWindow(), m_Window->GetContext());

	//Initialize Framebuffers & Viewports
	GPU::FrameBufferSpecification spec;
	spec.Attachments = { GPU::TextureFormat::RGB8, GPU::TextureFormat::Int32 };
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

bool CupOfTea::onScroll(int32 dx, int32 dy)
{
	auto& ortho = EditorCameraEntity.GetComponent<OrthographicCameraComponent>();
	ortho.ZoomLevel -= dy * 0.25f;
	ortho.ZoomLevel = std::max(ortho.ZoomLevel, 0.25f);

	auto& cam = EditorCameraEntity.GetComponent<CameraComponent>();

	glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
	box *= glm::vec2(cam.AspectRatio, 1.0f);
	cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
	cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;

	return true;
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

		//Load camera's Speed
		Entity entity = m_ActiveScene->GetSceneEntity();
		auto& sceneProp = entity.GetComponent<ScenePropertiesComponent>();
		m_CameraVelocity = sceneProp.CamVelocity;
	}
}

void CupOfTea::SaveSceneAs(void)
{
	std::string filepath = CreateFileDialog(FileDialogType::Save, "Green Tea Scene (*.gtscene)\0*.gtscene\0");
	if (!filepath.empty()) {
		if (filepath.find(".gtscene") == std::string::npos)
			filepath += ".gtscene";
		
		//Update Camera's Speed before saving
		Entity entity = m_ActiveScene->GetSceneEntity();
		auto& sceneProp = entity.GetComponent<ScenePropertiesComponent>();
		sceneProp.CamVelocity = m_CameraVelocity;
		m_ActiveScene->Save(filepath.c_str());
	}
}