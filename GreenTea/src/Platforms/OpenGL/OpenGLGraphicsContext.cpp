#include "OpenGLGraphicsContext.h"
#include <glad/glad.h>

namespace gte::GPU::OpenGL {

	void OpenGLGraphicsContext::Init(void* window) noexcept
	{
		mWindow = static_cast<GLFWwindow*>(window);
		glfwMakeContextCurrent(mWindow);
		
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ENGINE_ASSERT(status, "Failed to initialize Glad!");

		GTE_INFO_LOG("OpenGL Info:");
		GTE_INFO_LOG("  Vendor: ", glGetString(GL_VENDOR));
		GTE_INFO_LOG("  Renderer: ", glGetString(GL_RENDERER));
		GTE_INFO_LOG("  Version: ", glGetString(GL_VERSION));

		ENGINE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "GreenTea requires at least OpenGL version 4.5!");
	}

	void OpenGLGraphicsContext::SwapBuffers(void) noexcept { glfwSwapBuffers(mWindow); }

}