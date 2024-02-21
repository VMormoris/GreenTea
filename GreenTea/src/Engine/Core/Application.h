#pragma once

#include "Engine.h"
#include "Window.h"

namespace gte {

	class ENGINE_API Application {
	public:
		Application(const WindowSpecification& spec = {});
		virtual ~Application(void);

		void Run(void);

		bool Close(void) noexcept;

	protected:

		virtual void Update(float dt);

	protected:

		Window* mWindow = nullptr;

	private:

		bool mRunning = false;
		
	};

}

gte::Application* CreateApplication(int argc, char** argv);

/*
ImVec4* colors = ImGui::GetStyle().Colors;

ImVec4* colors = ImGui::GetStyle().Colors;


*/