#pragma once

#include <GreenTea.h>

namespace GTE {

	class SceneManagerPanel {
	public:

		SceneManagerPanel() = default;
		SceneManagerPanel(Scene* scene);

		void Render(void);

		void SetContext(Scene* scene);
		void SetSelectedEntity(Entity entity);
		Entity GetSelectedEntity(void) const;

	private:

		void RenderEntityNode(Entity entity);

	private:

		Entity m_SelectedEntity = { entt::null, nullptr };
		Scene* m_Context = nullptr;
	};

}