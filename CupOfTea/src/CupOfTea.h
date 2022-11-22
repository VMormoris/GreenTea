#pragma once

#include <GreenTea.h>
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ConsoleLogPanel.h"
#include "Panels/AnimationPanel.h"

class CupOfTea : public gte::Application {
public:
	CupOfTea(const std::string& filepath);
	~CupOfTea(void);
	virtual void Update(float) override;

private:

	void RenderGUI(void);

	void NewScene(void);
	void OpenScene(const std::filesystem::path& path);
	void SaveScene(void);
	void SaveSceneAs(const std::filesystem::path& path);

	bool DrawGuizmo(void);

	void OnOverlayRenderer(void);

	void Start(void);
	void Stop(void);

	bool OnKeyDown(gte::KeyCode keycode);
	bool OnScroll(float dx, float dy);

private:
	gte::Scene* mSnapshot = nullptr;
	std::string mScenePath = "";
	gte::gui::ImGuiLayer* gui = nullptr;

	gte::SceneHierarchyPanel mSceneHierarchyPanel;
	ContentBrowserPanel mBrowserPanel;
	ConsoleLogPanel mConsolePanel;
	AnimationPanel mAnimationPanel;

	//gte::GPU::FrameBuffer* mViewportFBO = nullptr;

	bool mPanels[8] = { true/*Viewport*/, true/*Content Browser*/, true/*Hierarchy*/, true/*Properties*/, true/*Console Log*/, false/*About*/, true/*Settings*/, false/*Animation Window*/};
	bool mShowColliders = false;
};