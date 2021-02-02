#pragma once

#include <GreenTea/GPU/Mesh.h>

#include <GreenTea/Scene/Components.h>

namespace GTE {

	struct SceneRenderData {
		glm::mat4 EyeMatrix = glm::mat4(1.0f);
		glm::vec3 CameraPos = {0.0f, 0.0f, 0.0f};
		glm::vec3 CameraDir = {0.0f, 0.0f, -1.0f};
		glm::vec3 LightPos = { 0.0f, 0.0f, 0.0f };
		LightComponent* lc = nullptr;
	};

	class ENGINE_API Renderer3D {
	public:

		static void Init(void);

		static void Shutdown(void);

		static void BeginShadowmap(const SceneRenderData& data);

		static void SubmitGeometry(const glm::mat4& transform, GPU::Mesh* mesh);

		static void EndShadowmap();

		static void BeginScene(const SceneRenderData& data);

		static void SubmitMesh(const glm::mat4& transform, GPU::Mesh* mesh, uint32 ID);

		static void EndScene();

	};

}