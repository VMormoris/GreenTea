#include "Panels.h"

#include <gtc/matrix_transform.hpp>

#include <imgui_internal.h>

namespace GTE {

	void RenderPropertiesPanel(Entity entity)
	{
		DrawComponentAdder(entity);
		
		DrawComponent<TagComponent>("Tag Component", entity, [&](auto& tag) {
			UISettings settings;
			DrawTextInput("Tag", tag.Tag, 64, settings);
		});

		DrawComponent<TransformComponent>("Transform Component", entity, [&](auto& transform) {
			UISettings settings;
			bool changed = DrawVec3Control("Position", transform.Position, settings);
			settings.ResetValue = 1.0f;
			if (DrawVec3Control("Scale", transform.Scale, settings))
				changed = true;
			settings.ResetValue = 0.0f;
			if (DrawVec3Control("Rotation", transform.Rotation, settings))
				changed = true;
			if (changed)
				entity.UpdateMatrices();
		});

		DrawComponent<MeshComponent>("Mesh Component", entity, [](auto& mc){
			UISettings settings;
			if (DrawFilePicker("Mesh", mc.Filepath, ".obj", settings))
				mc.Mesh = AssetManager::RequestMesh(mc.Filepath.c_str());
			if (mc.Mesh->Type == AssetType::MESH)
			{
				//Draw Material Inspector Panel
				GPU::Mesh* gpumesh = static_cast<GPU::Mesh*>(mc.Mesh->ActualAsset);
				RenderMaterialsPanel(gpumesh->GetMaterials(), mc.MaterialIndex);
			}
		});

		DrawComponent<Renderable2DComponent>("Renderable 2D Component", entity, [](auto& renderable) {
			
			UISettings settings;
			DrawColorPicker("Tint Color", renderable.Color, settings);
			if (DrawFilePicker("Texture", renderable.Filepath, ".png", settings))
				renderable.Texture = AssetManager::RequestTexture(renderable.Filepath.c_str());
			if (renderable.Texture->Type == AssetType::TEXTURE)
			{
				ImGui::PushID("Flip");
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, settings.ColumnWidth);
				ImGui::Text("Flip");
				ImGui::NextColumn();
				
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1.0f, 1.0f });
				ImGui::Text("");
				const float offset = ImGui::GetContentRegionAvailWidth() - (2 * ImGui::GetFrameHeight() + ImGui::CalcTextSize("XY").x + 4.0f);
				ImGui::SameLine(0.0f, offset);
				ImGui::Text("X");
				ImGui::SameLine();
				ImGui::Checkbox("##x", &renderable.FlipX);
				ImGui::SameLine();

				ImGui::Text("Y");
				ImGui::SameLine();
				ImGui::Checkbox("##y", &renderable.FlipY);

				ImGui::PopStyleVar();

				ImGui::Columns(1);
				ImGui::PopID();

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
				if (ImGui::TreeNodeEx("Texture Coordinates", ImGuiTreeNodeFlags_SpanAvailWidth))
				{
					GPU::Texture2D* texture = (GPU::Texture2D*)renderable.Texture->ActualAsset;
					DrawTextureCoordinates(renderable.TextCoords, texture->GetWidth(), texture->GetHeight(), settings);
					ImGui::TreePop();
				}
				ImGui::PopStyleVar();
			}
		});

		DrawComponent<CameraComponent>("Camera Component", entity, [&entity](auto& cam) {
			UISettings settings;
			settings.ColumnWidth = 125.0f;
			
			auto& persp = entity.GetComponent<PerspectiveCameraComponent>();
			const bool TargetFlag = DrawVec3Control("Target", persp.Target, settings);
			const bool UpVectorFlag = DrawVec3Control("Up Vector", persp.UpVector, settings);
			const bool FoVFlag = DrawFloatControl("FoV", persp.FoV, settings);
			const bool NearFlag = DrawFloatControl("Near Plane", persp.Near, settings);
			const bool FarFlag = DrawFloatControl("Far Plane", persp.Far, settings);
			DrawCheckboxControl("Primary", cam.Primary, settings);
			DrawCheckboxControl("Fixed Aspect Ratio", cam.FixedAspectRatio, settings);
			
			if (TargetFlag || UpVectorFlag)
			{
				const auto& transformation = entity.GetComponent<TransformationComponent>();
				cam.ViewMatrix = glm::lookAt(glm::vec3(transformation.Transform[3]), persp.Target, persp.UpVector);
				cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
			}

			if (FoVFlag || NearFlag || FarFlag)
			{
				cam.ProjectionMatrix = glm::perspective(glm::radians(persp.FoV), cam.AspectRatio, persp.Near, persp.Far);
				cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
			}
			
		});

		DrawComponent<LightComponent>("Light Component", entity, [&](auto& lc) {
			UISettings settings;
			if (DrawVec3Control("Target", lc.Target, settings))
				entity.UpdateMatrices();
			settings.Clamp.x = -180.0f;
			settings.Clamp.y = 180.0f;
			DrawFloatControl("Umbra", lc.Umbra, settings);
			DrawFloatControl("Penumbra", lc.Penumbra, settings);
			settings.Clamp.x = 0.0f;
			settings.Clamp.y = FLT_MAX;
			DrawFloatControl("Shadow Bias", lc.ShadowMapBias, settings);
			settings.Clamp.x = 1.0f;
			DrawFloatControl("Intensity", lc.Intensity, settings);
			DrawFloatControl("Near", lc.Near, settings);
			DrawFloatControl("Far", lc.Far, settings);
			settings = UISettings();
			DrawColorPicker("Color", lc.Color, settings);
		});

		DrawComponent<NativeScriptComponent>("Native Script Component", entity, [](auto& nScript) {
			UISettings settings;
			DrawTextInput("Class Name", nScript.ClassName, 64, settings);
		});

	}


	void RenderLogPanel(void)
	{
		if (ImGui::Button("Clear")) { Logger::Clear(); }
		ImGui::BeginChild("Logging");

		const auto& logs = Logger::Get();
		for (auto it = logs.begin(); it != logs.end(); ++it)
		{
			if ((*it).first == Logger::Type::TRACE)ImGui::Text((*it).second.c_str());
			else if ((*it).first == Logger::Type::ERR)ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), (*it).second.c_str());
			else if ((*it).first == Logger::Type::INFO)ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), (*it).second.c_str());
			else if ((*it).first == Logger::Type::WARNING) ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), (*it).second.c_str());
		}
		ImGui::EndChild();
	}

	void RenderMaterialsPanel(std::vector<Material>& materials, int32& index)
	{
		if (ImGui::Begin("Material Inspector"))
		{
			int32 size = static_cast<int32>(materials.size());
			if (size)
			{
				const char** values = new const char*[size];
				for (int32 i = 0; i < size; i++)
					values[i] = materials[i].Name.c_str();
				UISettings settings;
				settings.ColumnWidth = 64.0f;
				DrawComboControl("Material", index, values, size, settings);
				DrawMaterialControl(values[index], materials[index], settings);
				delete[] values;
			}
		}
		ImGui::End();
	}

}