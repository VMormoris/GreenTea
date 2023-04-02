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

		void SetDirectory(const std::filesystem::path& dir) noexcept { mDirectory = dir; }

		void PasteTo(Entity source, Entity target = {});

	private:

		void DrawEntityNode(Entity entity);

		void DrawSystemsNode(void);
		void DrawSystemsComponents(Entity entity);

	private:

		Entity mSelectionContext;
		std::string mFilter;
		std::filesystem::path mDirectory;

	};
}