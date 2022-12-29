#pragma once
#ifndef GT_DIST
#include <Engine/ImGui/ImGuiLayer.h>

namespace gte::GLFW {

	class ENGINE_API GLFWImGuiLayer : public gui::ImGuiLayer {
	public:

		void Init(void* window, GPU::GraphicsContext* context) override;

		void Begin(void) override;

		void End(void) override;

		void Shutdown(void) override;

		[[nodiscard]] ImGuiContext* GetContext(void) noexcept override;

	private:
		ImGuiContext* mContext = nullptr;
	};

}
#endif