#pragma once

#include <GreenTea/GPU/Mesh.h>

#include <GreenTea/Scene/Components.h>

#include <GreenTea/GPU/FrameBuffer.h>


namespace GTE {

	struct SceneData {
		glm::mat4 EyeMatrix = glm::mat4(1.0f);
		glm::vec3 CameraPos = { 0.0f, 0.0f, 0.0f };
		glm::vec3 CameraDir = { 0.0f, 0.0f, -1.0f };
		GPU::FrameBuffer* Target = nullptr;
	};

	struct LightSource {
		glm::vec3 Position;
		const LightComponent* lc = nullptr;

		LightSource(const glm::vec3& pos, const LightComponent& lightComponent)
			: Position(pos), lc(&lightComponent) {}
	};


	class Renderer {
	public:

		static void Init(void);

		static void Shutdown(void);

		static void BeginScene(const SceneData& data);

		static void SubmitMesh(const glm::mat4& transform, GPU::Mesh* mesh, uint32 ID);

		static void SubmitLight(const glm::vec3& position, const LightComponent& lc);

		static void EndScene(void);
	
	private:

		static void RenderGeometry(void);

		static void RenderShadowmaps(const LightSource& lc);

	};
}
