#pragma once

#include "Engine.h"
#include "Logger.h"

#include <Engine/Assets/ProjectManager.h>
#include <Engine/Assets/AssetManager.h>
#include <Engine/Core/Window.h>
#include <Engine/Core/DynamicLoader.h>
#include <Engine/Events/EventDispatcher.h>
#include <Engine/NativeScripting/ScriptingEngine.h>
#include <Engine/GPU/RendererAPI.h>
#include <Engine/Scene/Scene.h>


//Forward declaration(s)
namespace gte { class CollisionDispatcher; }

namespace gte::internal {

	/**
	* @brief GreenTea's Static context
	*/
	struct ENGINE_API Context {
		Logger logger;

		Window* GlobalWindow = nullptr;
		byte WindowCount = 0;
		glm::vec2 ViewportSize;
		EventDispatcher Dispatcher;
		
		ScriptingEngine* ScriptEngine = nullptr;
		DynamicLibLoader DynamicLoader;
		ProjectManager AssetWatcher;
		AssetManager AssetManager;

		Scene* ActiveScene = nullptr;

		GPU::GraphicsAPI G_API = GPU::GraphicsAPI::OpenGL;
		GPU::RendererAPI* Renderer = nullptr;

		CollisionDispatcher* CDispatcher = nullptr;
		bool Playing = false;
	};

	ENGINE_API Context* CreateContext(void) noexcept;
	ENGINE_API [[nodiscard]] Context* GetContext(void) noexcept;
	ENGINE_API void SetContext(Context* context) noexcept;
}