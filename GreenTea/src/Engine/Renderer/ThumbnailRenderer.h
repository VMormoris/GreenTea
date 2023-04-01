#pragma once

#include <Engine/Core/uuid.h>

#include <Engine/Assets/Geometry.h>
#include <Engine/Assets/Material.h>
#include <Engine/GPU/Framebuffer.h>

namespace gte {

	class ENGINE_API ThumbnailRenderer {
	public:
		
		static void Init(void);

		static void Shutdown(void);

		static void Render(Geometry* mesh, const std::vector<uuid>& materials, const glm::ivec2& size = { 128, 128 });

		static void Render(Geometry* mesh, const Material& mat, const glm::ivec2& size = { 128, 128 }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f });

		static GPU::FrameBuffer* GetThumbnail(void);

	};

}