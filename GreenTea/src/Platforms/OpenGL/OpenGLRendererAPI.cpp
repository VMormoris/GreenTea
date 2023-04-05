#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

void OpenGLMessageCallback(
	unsigned source,
	unsigned type,
	unsigned id,
	unsigned severity,
	int length,
	const char* message,
	const void* userParam)
{
	if (id == 131218)
		return;
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         GTE_FATAL_LOG('[', id, "]: ", message); return;
	case GL_DEBUG_SEVERITY_MEDIUM:       GTE_ERROR_LOG('[', id, "]: ", message); return;
	case GL_DEBUG_SEVERITY_LOW:          GTE_WARN_LOG('[', id, "]: ", message); return;
	case GL_DEBUG_SEVERITY_NOTIFICATION: GTE_INFO_LOG('[', id, "]: ", message); return;
	} 

	ENGINE_ASSERT(false, "Unknown severity level!");
}

namespace gte::GPU::OpenGL {

	void OpenGLRendererAPI::Init(void) noexcept
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(1.f);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_MULTISAMPLE);
	}

	void OpenGLRendererAPI::DrawIndexed(const VertexArray* va, uint32 indices) noexcept
	{
		va->Bind();
		glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL);
	}

	void OpenGLRendererAPI::DrawArrays(const VertexArray* va, uint32 vertices) noexcept
	{
		va->Bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices);
	}

	void OpenGLRendererAPI::DrawArrays(const VertexArray* va, uint32 offset, uint32 count) noexcept
	{
		va->Bind();
		glDrawArrays(GL_TRIANGLES, offset, count);
	}

	void OpenGLRendererAPI::DrawLines(const VertexArray* va, uint32 lines) noexcept
	{
		va->Bind();
		glDrawArrays(GL_LINES, 0, lines);
	}

	void OpenGLRendererAPI::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) noexcept { glViewport(x, y, width, height); }
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) noexcept { glClearColor(color.r, color.g, color.b, color.a); }
	void OpenGLRendererAPI::Clear(void) noexcept { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

	void OpenGLRendererAPI::SetLineThickness(float thickness) noexcept { glLineWidth(thickness); }
}