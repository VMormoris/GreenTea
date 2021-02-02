#include "RenderCommand.h"
#include "GreenTea/Core/Logger.h"

namespace GTE {
		
		GPU::RendererAPI* s_Renderer = nullptr;

		void RenderCommand::Init(void)
		{
			s_Renderer = GPU::RendererAPI::Create();
			s_Renderer->Init();
		}

		void RenderCommand::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height) { s_Renderer->SetViewport(x, y, width, height); }
		void RenderCommand::SetClearColor(const glm::vec4& color) { s_Renderer->SetClearColor(color); }
		void RenderCommand::Clear(void) { s_Renderer->Clear(); }
		void RenderCommand::DrawIndexed(const GPU::VertexArray* va, uint32 indices) { s_Renderer->DrawIndexed(va, indices); }
		void RenderCommand::DrawArray(const GPU::VertexArray* va, uint32 offset, uint32 count) { s_Renderer->DrawArray(va, offset, count); }
}