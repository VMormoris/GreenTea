#include "Context.h"

#include <Engine/Scene/CollisionDispatcher.h>

namespace gte::internal {

	static Context* sContext = nullptr;

	Context* CreateContext(void) noexcept
	{
		if (sContext)
			delete sContext;
		sContext = new Context;
		sContext->GreenTeaDir = std::filesystem::absolute(std::filesystem::current_path() / "../../..").string();
		std::replace(sContext->GreenTeaDir.begin(), sContext->GreenTeaDir.end(), '\\', '/');
		return sContext;
	}

	void SetContext(Context* context) noexcept
	{
		if (sContext == context)
			return;

		if (sContext)
			delete sContext;
		sContext = context;
	}

	[[nodiscard]] Context* GetContext(void) noexcept { return sContext; }

	void DestroyContext(Context* context) noexcept
	{
		delete context->CDispatcher;
		delete context->AudioDevice;
		delete context->ViewportFBO;
		delete context->Renderer;
		delete context->ActiveScene;
		delete context->ScriptEngine;
		delete context->GlobalWindow;

		delete context;
	}

}