#pragma once

#include <GreenTea.h>

class ProjectManagment : public gte::Application{
public:
	ProjectManagment(void);
	~ProjectManagment(void);

	void Update(float dt) override;

private:

	void RenderGUI(void);

private:

	gte::gui::ImGuiLayer* gui = nullptr;

};