#include "Shader.h"
#include "GraphicsContext.h"

#include <Platforms/OpenGL/OpenGLShader.h>

namespace gte::GPU {

	[[nodiscard]] Shader* Shader::Create(const char* shader_file) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLShader(shader_file);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}
}