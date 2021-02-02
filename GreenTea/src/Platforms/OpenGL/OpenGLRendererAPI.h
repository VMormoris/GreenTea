#ifndef _OPENGL_RENDERER_API
#define _OPENGL_RENDERER_API

#include "GreenTea/GPU/RendererAPI.h"

namespace GTE::GPU::OpenGL {

	class ENGINE_API OpenGLRendererAPI : public RendererAPI {
	public:
		void Init(void) override;
		void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear(void) override;
		void DrawIndexed(const VertexArray* va, uint32 indices) override;
		void DrawArray(const VertexArray* va, uint32 offset, uint32 count) override;
	};

}

#endif