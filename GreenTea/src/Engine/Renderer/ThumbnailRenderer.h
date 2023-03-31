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

		static void Render(Geometry* mesh, const std::vector<uuid>& materials);

		static void Render(Geometry* mesh, const Material& mat);

		static GPU::FrameBuffer* GetThumbnail(void);

	};

}