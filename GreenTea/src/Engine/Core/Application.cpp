#include "Application.h"
#include "Context.h"

#include <Engine/Renderer/RenderCommand.h>
#include <Engine/Renderer/Renderer2D.h>

#include <chrono>

#include <GLFW/glfw3.h>

namespace gte {

	void Application::Update(float dt) { }

	void Application::Run(void)
	{
		mRunning = true;
		auto simulation_start = std::chrono::steady_clock::now();

		while (mRunning)
		{
			auto simulation_end = std::chrono::steady_clock::now();
			float dt = std::chrono::duration <float>(simulation_end - simulation_start).count();// in seconds
			simulation_start = std::chrono::steady_clock::now();

			Update(dt);
			mWindow->Update();
		}
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