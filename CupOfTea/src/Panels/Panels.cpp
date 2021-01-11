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

		DrawComponent<Transform2DComponent>("Transform 2D Component", entity, [&](auto& transform) {
			UISettings settings;
			bool changed = DrawVec3Control("Position", transform.Position, settings);
			glm::vec3 value = glm::vec3(transform.Scale.x, transform.Scale.y, 1.0f);
			settings.ResetValue = 1.0f;
			settings.Disabled[2] = true;
			settings.Clamp = glm::vec2(0.0f, FLT_MAX);
			if (DrawVec3Control("Scale", value, settings))
			{
				transform.Scale = glm::vec2(value.x, value.y);
				changed = true;
			}
			value = glm::vec3(0.0f, 0.0f, transform.Rotation);
			settings.ResetValue = 0.0f;
			settings.Disabled[0] = true; settings.Disabled[1] = true; settings.Disabled[2]=false;
			settings.Clamp = glm::vec2(FLT_MIN, FLT_MAX);
			if (DrawVec3Control("Rotation", value, settings))
			{
				transform.Rotation = value.z;
				changed = true;
			}
			if (changed)
				entity.UpdateMatrices();
		});

		DrawComponent<Renderable2DComponent>("Renderable 2D Component", entity, [](auto& renderable) {
			
			UISettings settings;
			DrawColorPicker("Tint Color", renderable.Color, settings);
			if (DrawFilePicker("Texture", renderable.Filepath, settings))
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
			
			auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
			const bool ZoomFlag = DrawFloatControl("Zoom Level", ortho.ZoomLevel, settings);
			const bool BoundaryFlag = DrawFloatControl("Vetical Boundary", ortho.VerticalBoundary, settings);
			DrawCheckboxControl("Primary", cam.Primary, settings);
			DrawCheckboxControl("Fixed Aspect Ratio", cam.FixedAspectRatio, settings);
			if (ZoomFlag || BoundaryFlag)
			{
				glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
				box *= glm::vec2(cam.AspectRatio, 1.0f);
				cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
				cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
			}
		});

		DrawComponent<RigidBody2DComponent>("Rigidbody 2D Component", entity, [](auto& rigidbody) {
			static constexpr char* BodyTypes[] = { "Static", "Dynamic", "Kinematic" };
			UISettings settings;
			int32 index = (int32)rigidbody.Type;
			DrawComboControl("Body Type", index, BodyTypes, IM_ARRAYSIZE(BodyTypes), settings);
			rigidbody.Type = (BodyType)index;
			if (rigidbody.Type == BodyType::Static)
			{
				settings.Disabled[0] = true;
				settings.Disabled[1] = true;
			}
			DrawVec2Control("Velocity", rigidbody.Velocity, settings);
			DrawFloatControl("Ang. Velocity", rigidbody.AngularVelocity, settings);
			if (rigidbody.Type == BodyType::Kinematic)
			{
				settings.Disabled[0] = true;
			}
			settings.Speed = 1.0f;
			settings.Clamp = glm::vec2(0.0f, FLT_MAX);
			DrawFloatControl("Gravity Factor", rigidbody.GravityFactor, settings);
			DrawFloatControl("Mass", rigidbody.Mass, settings);
			DrawCheckboxControl("Fixed Rotation", rigidbody.FixedRotation, settings);
			DrawCheckboxControl("Bullet", rigidbody.Bullet, settings);
		});

		DrawComponent<CircleColliderComponent>("Circle Collider Component", entity, [](auto& collider) {
			UISettings settings;
			settings.Clamp = glm::vec2(0.0f, FLT_MAX);
			DrawFloatControl("Radius", collider.Radius, settings);
			DrawFloatControl("Friction", collider.Friction, settings);
			DrawFloatControl("Restitution", collider.Restitution, settings);
		});

		DrawComponent<BoxColliderComponent>("Box Collider Component", entity, [](auto& collider) {
			UISettings settings;
			settings.ResetValue = 1.0f;
			DrawVec2Control("Scale", collider.Scale, settings);
			settings.Clamp = glm::vec2(0.0f, FLT_MAX);
			DrawFloatControl("Friction", collider.Friction, settings);
			DrawFloatControl("Restitution", collider.Restitution, settings);
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

}