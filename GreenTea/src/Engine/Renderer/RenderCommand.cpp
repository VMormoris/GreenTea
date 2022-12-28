#include "RenderCommand.h"

#include <Engine/Core/Context.h>

namespace gte {

	void RenderCommand::Init(void)
	{
		internal::GetContext()->Renderer = GPU::RendererAPI::Create();
		internal::GetContext()->Renderer->Init();
	}

	void RenderCommand::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) { internal::GetContext()->Renderer->SetViewport(x, y, width, height); }
	void RenderCommand::SetClearColor(const glm::vec4& color) { internal::GetContext()->Renderer->SetClearColor(color); }
	void RenderCommand::Clear(void) { internal::GetContext()->Renderer->Clear(); }
	void RenderCommand::DrawIndexed(const GPU::VertexArray* va, uint32 indices) { internal::GetContext()->Renderer->DrawIndexed(va, indices); }
	void RenderCommand::DrawArrays(const GPU::VertexArray* va, uint32 vertices) { internal::GetContext()->Renderer->DrawArrays(va, vertices); }
	void RenderCommand::DrawLines(const GPU::VertexArray* va, uint32 lines) { internal::GetContext()->Renderer->DrawLines(va, lines); }

	void RenderCommand::SetLineThickness(float thickness) { internal::GetContext()->Renderer->SetLineThickness(thickness); }
}