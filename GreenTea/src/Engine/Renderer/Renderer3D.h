#pragma once

#include <Engine/GPU/Mesh.h>
#include <Engine/Scene/Components.h>
#include <Engine/GPU/Framebuffer.h>

namespace gte {

	struct ENGINE_API SceneData {
		glm::mat4 EyeMatrix = glm::mat4(1.0f);
		glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 ViewMatrix = glm::mat4(1.0f);
		glm::vec3 CameraPos = { 0.0f, 0.0f, 0.0f };
		glm::vec3 CameraDir = { 0.0f, 0.0f, -1.0f };
		GPU::FrameBuffer* Target = nullptr;
	};

	class ENGINE_API Renderer3D {
	public:

		static void Init(void);

		static void Shutdown(void);

		static void BeginScene(const SceneData& data);

		static void SubmitMesh(const glm::mat4& transform, GPU::Mesh* mesh, uint32 ID);

		static void SetSkybox(const GPU::Texture* Equirectangular, glm::ivec2 size);

		static void EndScene(void);

	private:

		static void RenderGeometry(void);

		static void RenderSkybox(void);

	};

}

