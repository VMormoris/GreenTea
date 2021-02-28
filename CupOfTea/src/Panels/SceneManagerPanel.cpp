#include "SceneManagerPanel.h"

namespace GTE {

	void SceneManagerPanel::Render(void)
	{
		//if (ImGui::BeginPopupContextItem())
		//{
		//	if (ImGui::MenuItem("Add Entity"))
		//		m_Context->CreateEntity();
		//	ImGui::EndPopup();
		//}

		auto view = m_Context->m_Registry.view<RelationshipComponent>();
		for (auto enttID : view)
		{
			const auto& rel = view.get(enttID);
			if (rel.Parent == entt::null)
				RenderEntityNode({ enttID, m_Context });
		}
		
	}

	void SceneManagerPanel::RenderEntityNode(Entity entity)
	{
		if (!entity.HasComponent<TagComponent>()) return;
		const auto& Tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags treeFlags = (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool IsOpen = ImGui::TreeNodeEx((void*)(uint64)&Tag, treeFlags, Tag.c_str());
		if (ImGui::IsItemClicked())
			m_SelectedEntity = entity;
		bool Destroy = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				if (m_SelectedEntity == entity)
					m_SelectedEntity = { entt::null, nullptr };
				Destroy = true;
			}
			if (ImGui::MenuItem("Clone Entity"))
			{
				Entity Clone = entity.Clone();
				Clone.UpdateMatrices();
			}
			if (ImGui::MenuItem("Add Child Entity"))
				entity.AddChild();
			ImGui::EndPopup();
		}
		if (IsOpen)
		{
			if (entity.Valid())
			{
				const auto& rel = entity.GetComponent<RelationshipComponent>();
				Entity Child{ rel.FirstChild, m_Context };
				size_t Childrens = rel.Childrens;
				for (size_t i = 0; i < Childrens; i++)
				{
					RenderEntityNode(Child);
					if (!Child.Valid())
						break;
					const auto& ChildRel = Child.GetComponent<RelationshipComponent>();
					Child = { ChildRel.Next, m_Context };
				}
			}
			ImGui::TreePop();
		}
		if (Destroy)
			entity.Destroy();
	}

	void SceneManagerPanel::SetContext(Scene* scene)
	{
		m_SelectedEntity = { entt::null, nullptr };
		m_Context = scene;
	}

	Entity SceneManagerPanel::GetSelectedEntity(void) const { return m_SelectedEntity; }


	SceneManagerPanel::SceneManagerPanel(Scene* scene)
		: m_Context(scene) {}

	void SceneManagerPanel::SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

}