#include "Shader.h"
#include "GraphicsContext.h"
#include "GreenTea/Core/Logger.h"

#include "Platforms/OpenGL/OpenGLShader.h"

namespace GTE::GPU {

	Shader* Shader::Create(const char* shader_file)
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