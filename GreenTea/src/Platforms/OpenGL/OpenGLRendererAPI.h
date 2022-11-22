#pragma once

#include <Engine/GPU/RendererAPI.h>

namespace gte::GPU::OpenGL {

	class ENGINE_API OpenGLRendererAPI : public RendererAPI {
	public:
		void Init(void) noexcept override;
		void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) noexcept override;
		void SetClearColor(const glm::vec4& color) noexcept override;
		void Clear(void) noexcept override;
		void DrawIndexed(const VertexArray* va, uint32 indices) noexcept override; 
		void DrawLines(const VertexArray* va, uint32 lines) noexcept override;
		void SetLineThickness(float thickness) noexcept override;
		void DrawFramebuffer(const FrameBuffer* fbo) noexcept override;
	};

}