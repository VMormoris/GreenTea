#pragma once
#include <Engine/GPU/GraphicsContext.h>
#include <GLFW/glfw3.h>

namespace gte::GPU::OpenGL {

	/**
	* @brief Class for representing Graphics Context on OpenGL API
	*/
	class ENGINE_API OpenGLGraphicsContext : public GraphicsContext {
	public:

		OpenGLGraphicsContext(void) = default;

		void Init(void* window) noexcept override;
		void SwapBuffers(void) noexcept override;

	private:
		GLFWwindow* mWindow = nullptr;
	};

}