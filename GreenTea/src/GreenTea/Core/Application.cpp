#include "Application.h"

#include "GreenTea/Events/EventDispatcher.h"
#include "GreenTea/Events/EventRegistry.h"

#include "GreenTea/Renderer/Renderer2D.h"
#include "GreenTea/Renderer/RenderCommand.h"

#include <chrono>

namespace GTE {

	void Application::Run(void)
	{
		auto simulation_start = std::chrono::steady_clock::now();
		const void* temp = m_Window->GetPlatformWindow();
		while (m_Running)
		{
			Event event;
			while (PollEvent(&event))
			{
				if (event.type == EventType::WindowClose)
					m_Running = false;
				else
				{
					if (event.category == EventCategory::KeyboardEvent)
					{
						if (!onKeyDown(event.key.code) && m_GameEvents)
							EventDispatcher::Dispatch(event);
					}
					else if (event.category == EventCategory::MouseEvent && m_GameEvents)
						EventDispatcher::Dispatch(event);
				}
			}
			
			auto simulation_end = std::chrono::steady_clock::now();
			float dt = std::chrono::duration <float>(simulation_end - simulation_start).count();// in seconds
			simulation_start = std::chrono::steady_clock::now();

			Update(dt);

			m_Window->Update();
		}
	}

	Application::Application(void)
	{
		m_Window = Window::Create("GreenTea App", 1280, 720);
		//m_Window->SetVSync(false);
		m_Running = true;
		REGISTER(EventType::WindowResize, this, &Application::onResize);
		
		//Prepare for Rendering
		RenderCommand::Init();
		Renderer2D::Init();
	}

	Application::Application(const char* title, int32 width, int32 height)
	{
		m_Window = Window::Create(title, width, height);
		//m_Window->SetVSync(false);
		m_Running = true;
		REGISTER(EventType::WindowResize, this, &Application::onResize);

		//Prepare for Rendering
		RenderCommand::Init();
		Renderer2D::Init();
	}

	Application::~Application(void)
	{
		UNREGISTER(this);
		delete m_Window;
	}

	bool Application::onResize(uint32 width, uint32 height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
		return false;
	}

	void Application::Update(float dt) {}

	bool Application::onKeyDown(KeyCode) { return false; }

}