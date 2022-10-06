#pragma once

#include <GreenTea.h>

namespace gte {
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;

		void Draw();
		void DrawComponents(Entity entity);

		[[nodiscard]] Entity GetSelectedEntity() const noexcept { return mSelectionContext; }
		void SetSelectedEntity(Entity entity) noexcept { mSelectionContext = entity; }

		void DeleteSelected(void);

	private:

		void DrawEntityNode(Entity entity);

	private:

		Entity mSelectionContext;
		std::string mFilter;

	};
}