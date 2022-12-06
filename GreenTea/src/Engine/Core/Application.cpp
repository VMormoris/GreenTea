#include "Application.h"
#include "Context.h"

#include <Engine/Renderer/RenderCommand.h>
#include <Engine/Renderer/Renderer2D.h>

#ifdef GT_WEB
	#include <emscripten/emscripten.h>
#endif

#include <chrono>

#include <GLFW/glfw3.h>


#ifdef GT_WEB
static void dispatch_main(void* fp)
{
	std::function<void()>* func = (std::function<void()>*)fp;
	(*func)();
}
#endif

namespace gte {

	void Application::Update(float dt) { }

	void Application::Run(void)
	{
		mRunning = true;
		auto simulation_start = std::chrono::steady_clock::now();
#ifdef GT_WEB
		std::function<void()> mainLoop = [&]() {
#else
		while (mRunning)
		{
#endif
			auto simulation_end = std::chrono::steady_clock::now();
			float dt = std::chrono::duration <float>(simulation_end - simulation_start).count();// in seconds
			simulation_start = std::chrono::steady_clock::now();

			Update(dt);
			mWindow->Update();

#ifdef GT_WEB
		};
		emscripten_set_main_loop_arg(dispatch_main, &mainLoop, 0, 1);
		mWindow->SetVSync(true);
#else
		}
#endif

	}

	Application::Application(const WindowSpecification& spec)
	{
		mWindow = Window::Create(spec);
		internal::GetContext()->GlobalWindow = mWindow;

		REGISTER(EventType::WindowClose, this, &Application::Close);

		RenderCommand::Init();
		Renderer2D::Init();

		auto* device = audio::AudioDevice::Get();//Create audio device
	}

	Application::~Application(void)
	{
		Renderer2D::Shutdown();
		UNREGISTER(this);
	}

	bool Application::Close(void) noexcept
	{
		mRunning = false;
		return true;
	}

}