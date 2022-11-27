#include "OpenGLRendererAPI.h"

#ifndef GT_WEB
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_ES31
	#include <GLFW/glfw3.h>
#endif

void OpenGLMessageCallback(
	unsigned source,
	unsigned type,
	unsigned id,
	unsigned severity,
	int length,
	const char* message,
	const void* userParam)
{
#ifndef GT_WEB
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         GTE_FATAL_LOG(message); return;
	case GL_DEBUG_SEVERITY_MEDIUM:       GTE_ERROR_LOG(message); return;
	case GL_DEBUG_SEVERITY_LOW:          GTE_WARN_LOG(message); return;
	case GL_DEBUG_SEVERITY_NOTIFICATION: GTE_INFO_LOG(message); return;
	}
#endif
	ENGINE_ASSERT(false, "Unknown severity level!");
}

namespace gte::GPU::OpenGL {

	void OpenGLRendererAPI::Init(void) noexcept
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef GT_WEB

		glEnable(GL_LINE_SMOOTH);

		glDebugMessageCallback(OpenGLMessageCallback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
	}

	void OpenGLRendererAPI::DrawIndexed(const VertexArray* va, uint32 indices) noexcept
	{
		va->Bind();
		glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL);
	}

	void OpenGLRendererAPI::DrawLines(const VertexArray* va, uint32 lines) noexcept
	{
		va->Bind();
		glDrawArrays(GL_LINES, 0, lines);
	}

	void OpenGLRendererAPI::DrawFramebuffer(const FrameBuffer* fbo) noexcept
	{
		const auto& spec = fbo->GetSpecification();
		fbo->ReadBind();
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, spec.Width, spec.Height, 0, 0, spec.Width, spec.Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	void OpenGLRendererAPI::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) noexcept { glViewport(x, y, width, height); }
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) noexcept { glClearColor(color.r, color.g, color.b, color.a); }
	void OpenGLRendererAPI::Clear(void) noexcept { glClear(GL_COLOR_BUFFER_BIT); }

	void OpenGLRendererAPI::SetLineThickness(float thickness) noexcept { glLineWidth(thickness); }
}