#include "SceneHierarchyPanel.h"

//#include <Engine/NativeScripting/ScriptableEntity.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <IconsForkAwesome.h>
#include <gtc/matrix_transform.hpp>

#include "ContentBrowserPanel.h"

namespace gte {


	void SceneHierarchyPanel::Draw(void)
	{		
		const float PanelWidth = ImGui::GetContentRegionAvail().x;
		Scene* scene = gte::internal::GetContext()->ActiveScene;

		gte::gui::DrawSearchbar("Entity search", mFilter, 64, PanelWidth - 66.0f);
		ImGui::SameLine();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		if (ImGui::Button("Add"))
			ImGui::OpenPopup("Add Entity");
		ImGui::PopStyleVar();
		
		if (ImGui::BeginPopup("Add Entity"))
		{
			if (ImGui::MenuItem("Add empty Entity"))
				scene->CreateEntity("Unnamed Entity");
			if (ImGui::MenuItem("Camera Entity"))
			{
				Entity entity = scene->CreateEntity("Camera");
				entity.AddComponent<Transform2DComponent>();
				entity.AddComponent<CameraComponent>();
			}
			ImGui::EndPopup();
		}
		
		ImGui::Separator();
		if (!gte::internal::GetContext()->ActiveScene) { ImGui::End(); return; }
		
		ImGui::BeginChild("##gte::Treeview");
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Add empty Entity"))
				scene->CreateEntity("Unnamed Entity");
			if (ImGui::MenuItem("Camera Entity"))
			{
				Entity entity = scene->CreateEntity("Camera");
				entity.AddComponent<Transform2DComponent>();
				entity.AddComponent<CameraComponent>();
			}
			ImGui::EndPopup();
		}
			
		if (mFilter.empty())
		{
			auto view = scene->mReg.view<RelationshipComponent>();
			for (auto entityID : view)
			{
				if (!scene->mReg.valid(entityID))
					continue;
				const auto [relc] = view.get(entityID);
				if (relc.Parent == entt::null)
					DrawEntityNode({ entityID, scene });
			}
		}
		else
		{
			auto view = scene->mReg.view<TagComponent>();
			for (auto entityID : view)
			{
				const auto [tag] = view.get(entityID);
				std::string Tag = "";
				std::for_each(tag.Tag.begin(), tag.Tag.end(), [&](const char c) { Tag += std::tolower(c); });
				if (Tag.find(mFilter) == std::string::npos)
					continue;
				DrawEntityNode({ entityID, scene });
			}
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			mSelectionContext = {};
		ImGui::EndChild();
		
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				std::filesystem::path filepath = (const char*)payload->Data;
				if (filepath.extension() == ".gtprefab")
				{
					uuid id = internal::GetContext()->AssetWatcher.GetID(filepath.string());
					Ref<Asset> prefab = CreateRef<Asset>(id);
					SpawnEntity(prefab);
				}
			}
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		Scene* scene = gte::internal::GetContext()->ActiveScene;
		const auto& tag = entity.GetComponent<TagComponent>().Tag;
		const auto& relc = entity.GetComponent<RelationshipComponent>();

		//Set flag and styles
		ImGuiTreeNodeFlags flags = ((mSelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		if (relc.Childrens == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool shouldDestroy = false;
		const bool opened = ImGui::TreeNodeEx((void*)(uint64)(uint32)entity, flags, tag.c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
			{
				uuid id = *(uuid*)payload->Data;
				Entity toMove = scene->FindEntityWithUUID(id);
				scene->MoveEntity(entity, toMove);
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				std::filesystem::path filepath = (const char*)payload->Data;
				if (filepath.extension() == ".gtprefab")
				{
					uuid id = internal::GetContext()->AssetWatcher.GetID(filepath.string());
					Ref<Asset> prefab = CreateRef<Asset>(id);
					SpawnEntity(entity, prefab);
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (ImGui::BeginDragDropSource())
		{
			uuid id = entity.GetID();
			ImGui::SetDragDropPayload("ENTITY_ITEM", (void*)&id, sizeof(uuid));
			ImGui::Text(entity.GetName().c_str());
			ImGui::EndDragDropSource();
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			mSelectionContext = entity;

		if (ImGui::BeginPopupContextItem())
		{
			constexpr char biggest[] = "Delete Entity Delete";
			if (ImGui::MenuItem("Add Child Entity"))
				scene->CreateChildEntity(entity);
			if (gte::gui::DrawMenuItem(ICON_FK_TRASH, "Delete Entity", "Delete", biggest))
				shouldDestroy = true;			
			if (gte::gui::DrawMenuItem(ICON_FK_CLONE, "Clone Entity", nullptr, biggest))
				scene->Clone(entity);
			if (gte::gui::DrawMenuItem(ICON_FK_CUBE, "Create Prefab", nullptr, biggest))
				CreatePrefab(entity, mDirectory);
			ImGui::EndPopup();
		}
		
		if (opened)//Render childs
		{
			Entity child = { relc.FirstChild, scene };
			size_t childrens = relc.Childrens;
			for (size_t i = 0; i < childrens; i++)
			{
				const auto nextID = child.GetComponent<RelationshipComponent>().Next;//Must be get before draw cause it might be deleted
				DrawEntityNode(child);
				if (nextID == entt::null)
					break;
				child = { nextID, scene };
			}
			ImGui::TreePop();
		}

		if (shouldDestroy)
		{
			uuid id;
			if(mSelectionContext)
				id = mSelectionContext.GetComponent<IDComponent>().ID;
			scene->DestroyEntity(entity);
			if(mSelectionContext)
				mSelectionContext = scene->FindEntityWithUUID(id);
		}

		if (mSelectionContext && !scene->mReg.valid(mSelectionContext))
			mSelectionContext = {};
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (!entity)
			return;
		static bool sTagEdit = false;

		ImGuiIO& io = ImGui::GetIO();
		auto IconsFont = io.Fonts->Fonts[3];

		//Draw Tag Component
		ImGui::PushFont(IconsFont);
		ImGui::Text(ICON_FK_TAG);
		ImGui::PopFont();
		ImGui::SameLine();
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		gui::DrawEditText(tag, 64, sTagEdit);
		const float offset = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Add").x + 4;
		ImGui::SameLine(offset);
		if (ImGui::Button("Add"))
			ImGui::OpenPopup("AddComponent");
		
		if (ImGui::BeginPopup("AddComponent"))
		{
			constexpr char biggest[] = "Circle Collider Component";
			if (!entity.HasComponent<Transform2DComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_CUBE, "Trasnform 2D Component", "", biggest))
				{
					entity.AddComponent<Transform2DComponent>();
					gte::internal::GetContext()->ActiveScene->UpdateTransform(entity);
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<SpriteRendererComponent>() && !entity.HasComponent<CircleRendererComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_PICTURE_O, "Sprite Renderer Component", "", biggest))
				{
					entity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (gui::DrawMenuItem(ICON_FK_CIRCLE, "Circle Renderer Component", "", biggest))
				{
					entity.AddComponent<CircleRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<CameraComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_CAMERA_RETRO, "Camera Component", "", biggest))
				{
					auto& cam = entity.AddComponent<CameraComponent>();
					const auto& ortho = entity.GetComponent<OrthographicCameraComponent>();

					gte::Window* window = gte::internal::GetContext()->GlobalWindow;
					cam.AspectRatio = static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight());
					if (!entity.HasComponent<Transform2DComponent>())
						entity.AddComponent<Transform2DComponent>();
					const auto& tc = entity.GetComponent<TransformationComponent>();
					cam.ViewMatrix = glm::inverse(tc.Transform);
					glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
					box *= glm::vec2(cam.AspectRatio, 1.0f);
					cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<Rigidbody2DComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_SKATE, "Rigidbody 2D Component", "", biggest))
				{
					entity.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<BoxColliderComponent>() && !entity.HasComponent<CircleColliderComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_SQUARE_O, "Box Collider Component", "", biggest))
				{
					entity.AddComponent<BoxColliderComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (gui::DrawMenuItem(ICON_FK_CIRCLE_O, "Circle Collider Component", "", biggest))
				{
					entity.AddComponent<CircleColliderComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<SpeakerComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_VOLUME_UP, "Speaker Component", "", biggest))
				{
					entity.AddComponent<SpeakerComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<ParticleSystemComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_SPINNER, "Particle System Component", "", biggest))
				{
					entity.AddComponent<ParticleSystemComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<NativeScriptComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_CODE, "Native Script Component", "", biggest))
				{
					entity.AddComponent<NativeScriptComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		//Draw ID Component
		ImGui::PushFont(IconsFont);
		ImGui::Text(ICON_FK_KEY_MODERN);
		ImGui::PopFont();
		ImGui::SameLine();
		const uuid& id = entity.GetComponent<IDComponent>().ID;
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::Text(id.str().c_str());
		ImGui::PopStyleVar();
		ImGui::Separator();

		//Draw all other Components
		if (entity.HasComponent<Transform2DComponent>())
		{
			gui::DrawComponent<Transform2DComponent>(ICON_FK_CUBE, "Transform 2D Component", entity, [&](auto& tc) {
				static int32 world = 0;
				//ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				//ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
				//ImGui::Button("##dummy", {0.0f, 0.0f});
				//ImGui::PopStyleColor();
				//ImGui::PopItemFlag();
				//ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 156.0f);
				//ImGui::Text("Local"); ImGui::SameLine(); ImGui::RadioButton("##local", &world, 0); ImGui::SameLine();
				//ImGui::Text("World"); ImGui::SameLine(); ImGui::RadioButton("##world", &world, 1);

				if (!world)
				{
					gui::UISettings settings;
					bool changed = gui::DrawVec3Control("Position", tc.Position, settings);
					glm::vec3 scale = { tc.Scale.x, tc.Scale.y, 1.0f };
					settings.MaxFloat = FLT_MAX;
					settings.Enabled[2] = false;
					settings.ResetValue = 1.0f;
					if (gui::DrawVec3Control("Scale", scale, settings))
					{
						tc.Scale = { scale.x, scale.y };
						changed = true;
					}

					settings.MinFloat = -180.0;
					settings.MaxFloat = 180.0;
					settings.Enabled = { false, false, true, false };
					settings.ResetValue = 0.0f;
					glm::vec3 rotation{ 0.0f, 0.0f, tc.Rotation };
					if (gui::DrawVec3Control("Rotation", rotation, settings))
					{
						tc.Rotation = rotation.z;
						changed = true;
					}

					if (changed)
						internal::GetContext()->ActiveScene->UpdateTransform(entity);
				}
				else
				{
					gui::UISettings settings;
					glm::vec3 pos, scale, rotation;
					auto& transformation = entity.GetComponent<TransformationComponent>();
					math::DecomposeTransform(transformation, pos, scale, rotation);
					bool changed = gui::DrawVec3Control("Position", pos, settings);
					settings.MaxFloat = FLT_MAX;
					settings.Enabled[2] = false;
					settings.ResetValue = 1.0f;
					if (gui::DrawVec3Control("Scale", scale, settings))
						changed = true;
					settings.MinFloat = -180.0;
					settings.MaxFloat = 180.0;
					settings.Enabled = { false, false, true, false };
					settings.ResetValue = 0.0f;
					rotation = glm::degrees(rotation);
					if (gui::DrawVec3Control("Rotation", rotation, settings))
						changed = true;

					if (changed)
					{
						transformation = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), scale);
						const auto& rel = entity.GetComponent<RelationshipComponent>();
						bool found = false;
						glm::mat4 pTransformation;
						if (rel.Parent != entt::null)
						{
							Scene* scene = gte::internal::GetContext()->ActiveScene;
							Entity parent = { rel.Parent, scene };
							
							if (parent.HasComponent<TransformationComponent>())
							{
								found = true;
								pTransformation = parent.GetComponent<TransformationComponent>();
							}
							else
							{
								auto pID = parent.GetComponent<RelationshipComponent>().Parent;
								parent = { pID, scene };
								while (pID != entt::null || !parent.HasComponent<TransformationComponent>())
								{
									pID = parent.GetComponent<RelationshipComponent>().Parent;
									parent = { pID, scene };
								}
								if (pID != entt::null)
								{
									found = true;
									pTransformation = parent.GetComponent<TransformationComponent>();
								}
							}
						}

						if (found)
							transformation = glm::inverse(pTransformation) * transformation.Transform;
						
						math::DecomposeTransform(transformation, pos, scale, rotation);
						tc.Position = pos;
						tc.Scale = { scale.x, scale.y };
						tc.Rotation = glm::degrees(rotation.z);
					}
				}
			});
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			gui::DrawComponent<SpriteRendererComponent>(ICON_FK_PICTURE_O, "Sprite Renderer Component", entity, [](auto& sprite) {
				gui::UISettings settings;
				uuid id = sprite.Texture->ID;
				if (gui::DrawAssetControl("Texture", id, ".gtimg", settings))
					sprite.Texture = internal::GetContext()->AssetManager.RequestAsset(id);
				settings.ResetValue = 1.0f;
				gui::DrawColorPicker("Tint Color", sprite.Color, settings);
				if (sprite.Texture->Type == AssetType::TEXTURE)
				{
					GPU::Texture* texture = (GPU::Texture*)sprite.Texture->Data;
					settings.MinFloat = 1.0f;
					settings.MaxFloat = FLT_MAX;
					gui::DrawFloatControl("Tiling Factor", sprite.TilingFactor, settings);
					gui::DrawVec2BoolControl("Flip", sprite.FlipX, sprite.FlipY, settings);
				}
				gui::DrawBoolControl("Visible", sprite.Visible, settings);
				if(sprite.Texture->Type == AssetType::TEXTURE)
				{
					GPU::Texture* texture = (GPU::Texture*)sprite.Texture->Data;
					ImGui::SetCursorPosX(0.0f);
					if (ImGui::TreeNodeEx("Texture Coordinates", ImGuiTreeNodeFlags_OpenOnArrow))
					{
						gui::DrawTextureCoordinates(sprite.Coordinates, texture->GetWidth(), texture->GetHeight(), settings);
						ImGui::TreePop();
					}
				}
			});
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			gui::DrawComponent<CircleRendererComponent>(ICON_FK_CIRCLE, "Circle Renderer Component", entity, [](auto& circle) {
				gui::UISettings settings;
				gui::DrawColorPicker("Color", circle.Color, settings);
				settings.MaxFloat = 1.0f;
				gui::DrawFloatControl("Thickness", circle.Thickness, settings);
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Fade", circle.Fade, settings);
				gui::DrawBoolControl("Visible", circle.Visible, settings);
			});
		}

		if (entity.HasComponent<CameraComponent>())
		{
			auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
			gui::DrawComponent<CameraComponent>(ICON_FK_CAMERA_RETRO, "Camera Component", entity, [&](auto& cam) {
				gui::UISettings settings;
				settings.MinFloat = 0.25f;
				settings.MaxFloat = FLT_MAX;
				bool changed = gui::DrawFloatControl("Zoom Level", ortho.ZoomLevel, settings);
				settings.MinFloat = 0.5f;
				if (gui::DrawFloatControl("Vertical Boundary", ortho.VerticalBoundary, settings))
					changed = true;
				gui::DrawBoolControl("Primary", cam.Primary, settings);
				if (gui::DrawBoolControl("Fixed Asp. Ratio", cam.FixedAspectRatio, settings) && !cam.FixedAspectRatio)
					cam.AspectRatio = static_cast<float>(internal::GetContext()->ViewportSize.x) / static_cast<float>(internal::GetContext()->ViewportSize.y);
				settings.MinFloat = 0.1f;
				settings.MaxFloat = FLT_MAX;
				if (cam.FixedAspectRatio)
				{
					if (gui::DrawFloatControl("Aspect Ratio", cam.AspectRatio, settings))
						changed = true;
				}

				ImGui::SetCursorPosX(0.0f);
				if (ImGui::TreeNodeEx("Audio Listener", ImGuiTreeNodeFlags_OpenOnArrow))
				{
					static constexpr char* typestr[] = { "None", "Inverse", "Inverse Clamp", "Linear", "Linear Clamp", "Exponent", "Exponent Clamp" };
					int32 index = static_cast<int32>(cam.Model);
					if (gui::DrawComboControl("Distance Model", index, typestr, 7, settings))
						cam.Model = static_cast<DistanceModel>(index);
					settings.MinFloat = 0.0f;
					settings.MaxFloat = 1.0f;
					gui::DrawFloatControl("Master Volume", cam.MasterVolume, settings);
					ImGui::TreePop();
				}

				if (changed)
				{
					glm::vec2 box = glm::vec2(ortho.VerticalBoundary * ortho.ZoomLevel);
					box *= glm::vec2(cam.AspectRatio, 1.0f);
					cam.ProjectionMatrix = glm::ortho(-box.x, box.x, -box.y, box.y, -1.0f, 1.0f);
					cam.EyeMatrix = cam.ProjectionMatrix * cam.ViewMatrix;
				}
			});
		}

		if (entity.HasComponent<ParticleSystemComponent>())
		{
			gui::DrawComponent<ParticleSystemComponent>(ICON_FK_SPINNER, "Particle System Component", entity, [](auto& psc) {
				gui::UISettings settings;

				gui::DrawColorPicker("ColorBegin", psc.Props.ColorBegin, settings);
				gui::DrawColorPicker("ColorEnd", psc.Props.ColorEnd, settings);

				gui::DrawVec2Control("Position", psc.Props.Position, settings);
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawVec2Control("Size Begin", psc.Props.SizeBegin, settings);
				gui::DrawVec2Control("Size End", psc.Props.SizeEnd, settings);
				settings.MaxFloat = 0.0f;
				gui::DrawVec2Control("Velocity", psc.Props.Velocity, settings);
				gui::DrawVec2Control("Velocity Variation", psc.Props.VelocityVariation, settings);
				settings.MinFloat = -180.0f;
				settings.MaxFloat = 180.0f;
				gui::DrawFloatControl("Rotation", psc.Props.Rotation, settings);
				gui::DrawFloatControl("Ang. Velocity", psc.Props.AngularVelocity, settings);
				settings.MinFloat = 0.0f;
				gui::DrawFloatControl("Ang. Vel. Variation", psc.Props.AngularVelocityVariation, settings);
				gui::DrawFloatControl("Duration", psc.Props.Duration, settings);
				gui::DrawFloatControl("Life Time", psc.Props.LifeTime, settings);
				gui::DrawFloatControl("Emition Rate", psc.Props.EmitionRate, settings);
				settings.MinUint = 0;
				settings.MaxUint = std::numeric_limits<uint32>::max();
				gui::DrawUint32Control("Max Particles", psc.Props.MaxParticles, settings);
				gui::DrawBoolControl("Looping", psc.Props.Looping, settings);
			});
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			gui::DrawComponent<Rigidbody2DComponent>(ICON_FK_SKATE, "Rigidbody 2D Component", entity, [](auto& rb) {
				static constexpr char* typestr[3] = { "Static", "Dynamic", "Kinematic" };
				gui::UISettings settings;
				int32 index = static_cast<int32>(rb.Type);
				if (gui::DrawComboControl("Body Type", index, typestr, 3, settings))
					rb.Type = static_cast<BodyType>(index);
				gui::DrawVec2Control("Velocity", rb.Velocity, settings);
				gui::DrawFloatControl("Ang. Velocity", rb.AngularVelocity, settings);
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Gravity Factor", rb.GravityFactor, settings);
				gui::DrawBoolControl("Fixed Rotation", rb.FixedRotation, settings);
				gui::DrawBoolControl("Bullet", rb.Bullet, settings);
			});
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			gui::DrawComponent<BoxColliderComponent>(ICON_FK_SQUARE_O, "Box Collider Component", entity, [](auto& bc) {
				gui::UISettings settings;
				gui::DrawVec2Control("Offset", bc.Offset, settings);
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawVec2Control("Size", bc.Size, settings);
				gui::DrawFloatControl("Density", bc.Density, settings);
				gui::DrawFloatControl("Friction", bc.Friction, settings);
				gui::DrawFloatControl("Restitution", bc.Restitution, settings);
				gui::DrawFloatControl("Rest. Threshold", bc.RestitutionThreshold, settings);
				gui::DrawBoolControl("Sensor", bc.Sensor, settings);
			});
		}

		if (entity.HasComponent<CircleColliderComponent>())
		{
			gui::DrawComponent<CircleColliderComponent>(ICON_FK_CIRCLE_O, "Circle Collider Component", entity, [](auto& cc) {
				gui::UISettings settings;
				gui::DrawVec2Control("Offset", cc.Offset, settings);
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Radius", cc.Radius, settings);
				gui::DrawFloatControl("Density", cc.Density, settings);
				gui::DrawFloatControl("Friction", cc.Friction, settings);
				gui::DrawFloatControl("Restitution", cc.Restitution, settings);
				gui::DrawFloatControl("Rest. Threshold", cc.RestitutionThreshold, settings);
				gui::DrawBoolControl("Sensor", cc.Sensor, settings);
			});
		}

		if (entity.HasComponent<SpeakerComponent>())
		{
			gui::DrawComponent<SpeakerComponent>(ICON_FK_VOLUME_UP, "Speaker Component", entity, [](auto& speaker) {
				gui::UISettings settings;
				uuid id = speaker.AudioClip->ID;
				if (gui::DrawAssetControl("Audio Clip", id, ".gtaudio", settings))
					speaker.AudioClip = internal::GetContext()->AssetManager.RequestAsset(id);
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Volume", speaker.Volume, settings);
				gui::DrawFloatControl("Pitch", speaker.Pitch, settings);
				gui::DrawFloatControl("Roll off factor", speaker.RollOffFactor, settings);
				gui::DrawFloatControl("Ref. Distance", speaker.RefDistance, settings);
				gui::DrawFloatControl("Max Distance", speaker.MaxDistance, settings);
				gui::DrawBoolControl("Looping", speaker.Looping, settings);
			});
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			gui::DrawComponent<NativeScriptComponent>(ICON_FK_CODE, "Native Script Component", entity, [](auto& nc) {
				gui::UISettings settings;
				uuid id = nc.ScriptAsset->ID;
				nc.ScriptAsset = internal::GetContext()->AssetManager.RequestAsset(id);
				if (gui::DrawAssetControl("Script", id, ".gtscript", settings))
				{
					nc.Description = internal::NativeScript();
					nc.ScriptAsset = internal::GetContext()->AssetManager.RequestAsset(id);
					if(id.IsValid())
						nc.Description = *((internal::NativeScript*)nc.ScriptAsset->Data);
				}

				const auto& fieldsSpec = nc.Description.GetFieldsSpecification();
				void* buffer = gte::internal::GetContext()->Playing ? nc.Instance : nc.Description.GetBuffer();
				//void* buffer = nc.Description.GetBuffer();
				for (const auto& spec : fieldsSpec)
					gui::DrawFieldControl(spec, buffer);
			});
		}

	}

	void SceneHierarchyPanel::DeleteSelected(void)
	{
		if (!mSelectionContext)
			return;
		gte::internal::GetContext()->ActiveScene->DestroyEntity(mSelectionContext);
		mSelectionContext = {};
	}

}