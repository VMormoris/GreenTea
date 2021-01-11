#ifndef CUP_OF_TEA
#define CUO_OF_TEA

#include <GreenTea.h>

#include "Panels/SceneManagerPanel.h"

struct ProjectProperties {
	std::string ProjectPath;
	std::string ProjectName;
	std::string WorkingScene;
};

using namespace GTE;

class CupOfTea : public Application {
public:
	
	CupOfTea(const char* filepath = nullptr);
	~CupOfTea(void);

	void Update(float dt) override;

public:

	bool onScroll(int32 dx, int32 dy);
	bool onKeyDown(KeyCode keycode) override;

private:

	void NewScene(void);
	void OpenScene(void);
	void SaveSceneAs(void);

private:

	ImGuiLayer* m_EditorLayer = nullptr;

	GPU::FrameBuffer* m_ViewportFBO = nullptr;
	GPU::FrameBuffer* m_CamFBO = nullptr;

	glm::vec2 m_ViewportSize;
	glm::vec2 m_CamViewportSize;
	glm::vec2 m_CameraVelocity = glm::vec2{ 1.0f, 1.0f };

	bool m_Playing = false;

	bool m_Panels[5] = {true , true, true, true, true};
	SceneManagerPanel m_SceneManagerPanel;

	Scene* m_ActiveScene;

};


#endif