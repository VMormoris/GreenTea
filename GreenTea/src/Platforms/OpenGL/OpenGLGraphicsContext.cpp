#include "OpenGLGraphicsContext.h"

#ifndef GT_WEB
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_ES31
	#include <GLFW/glfw3.h>
#endif

namespace gte::GPU::OpenGL {

	void OpenGLGraphicsContext::Init(void* window) noexcept
	{
		mWindow = static_cast<GLFWwindow*>(window);
		glfwMakeContextCurrent(mWindow);
#ifndef GT_WEB 
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ENGINE_ASSERT(status, "Failed to initialize Glad!");
		
		GTE_INFO_LOG("OpenGL Info:");
		GTE_INFO_LOG("  Vendor: ", glGetString(GL_VENDOR));
		GTE_INFO_LOG("  Renderer: ", glGetString(GL_RENDERER));
		GTE_INFO_LOG("  Version: ", glGetString(GL_VERSION));
#endif

#ifdef GT_WEB
		ENGINE_ASSERT(GLVersion.major > 3 || (GLVersion.major == 3 && GLVersion.minor >= 0), "GreenTea for Web requires at least OpenGL version 3.0!");
#else
		ENGINE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "GreenTea requires at least OpenGL version 4.5!");
#endif
	}

	void OpenGLGraphicsContext::SwapBuffers(void) noexcept { glfwSwapBuffers(mWindow); }

}