#include "OpenGLRendererAPI.h"
#include "GreenTea/Core/Logger.h"

#include <GL/glew.h>

void OpenGLMessageCallback(
	unsigned source,
	unsigned type,
	unsigned id,
	unsigned severity,
	int length,
	const char* message,
	const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         GTE_FATAL_LOG(message); return;
	case GL_DEBUG_SEVERITY_MEDIUM:       GTE_ERROR_LOG(message); return;
	case GL_DEBUG_SEVERITY_LOW:          GTE_WARN_LOG(message); return;
	case GL_DEBUG_SEVERITY_NOTIFICATION: GTE_INFO_LOG(message); return;
	}

	ENGINE_ASSERT(false, "Unknown severity level!");
}

namespace GTE::GPU::OpenGL {

	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDebugMessageCallback(OpenGLMessageCallback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	}

	void OpenGLRendererAPI::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) { glViewport(x, y, width, height); }
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.b, color.a); }
	void OpenGLRendererAPI::Clear(void) { glClear(GL_COLOR_BUFFER_BIT); }
	void OpenGLRendererAPI::DrawIndexed(const VertexArray* va, uint32 indices) { glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL); }

}