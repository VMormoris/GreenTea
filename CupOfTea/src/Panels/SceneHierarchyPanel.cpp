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
				mSelectionContext = scene->CreateEntity("Unnamed Entity");
			if (ImGui::MenuItem("Camera Entity"))
			{
				Entity entity = scene->CreateEntity("Camera");
				entity.AddComponent<Transform2DComponent>();
				entity.AddComponent<CameraComponent>();
				mSelectionContext = entity;
			}
			ImGui::EndPopup();
		}
		
		ImGui::Separator();
		if (!gte::internal::GetContext()->ActiveScene) { ImGui::End(); return; }
		
		ImGui::BeginChild("##gte::Treeview");
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Add empty Entity"))
				mSelectionContext = scene->CreateEntity("Unnamed Entity");
			if (ImGui::MenuItem("Camera Entity"))
			{
				Entity entity = scene->CreateEntity("Camera");
				entity.AddComponent<Transform2DComponent>();
				entity.AddComponent<CameraComponent>();
				mSelectionContext = entity;
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
				mSelectionContext = scene->CreateChildEntity(entity);
			if (gte::gui::DrawMenuItem(ICON_FK_TRASH, "Delete Entity", "Delete", biggest))
				shouldDestroy = true;			
			if (gte::gui::DrawMenuItem(ICON_FK_CLONE, "Clone Entity", nullptr, biggest))
				mSelectionContext = scene->Clone(entity);
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
			scene->DestroyEntity(entity);
			if(!mSelectionContext)
				mSelectionContext = {};
		}

	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (!entity)
			return;

		if (!entity.HasComponent<TagComponent>())
		{
			mSelectionContext = {};
			return;
		}
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
				if (gui::DrawMenuItem(ICON_FK_CUBE, "Trasnform 2D Component", nullptr, biggest))
				{
					entity.AddComponent<Transform2DComponent>();
					gte::internal::GetContext()->ActiveScene->UpdateTransform(entity);
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<SpriteRendererComponent>() && !entity.HasComponent<CircleRendererComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_PICTURE_O, "Sprite Renderer Component", nullptr, biggest))
				{
					entity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (gui::DrawMenuItem(ICON_FK_CIRCLE, "Circle Renderer Component", nullptr, biggest))
				{
					entity.AddComponent<CircleRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<AnimationComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_FILM, "Animation Component", nullptr, biggest))
				{
					entity.AddComponent<AnimationComponent>();
					if (entity.HasComponent<CircleRendererComponent>())
						entity.RemoveComponent<CircleRendererComponent>();
					if (!entity.HasComponent<SpriteRendererComponent>())
						entity.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<TextRendererComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_FONT, "Text Renderer Component", nullptr, biggest))
				{
					entity.AddComponent<TextRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<CameraComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_CAMERA_RETRO, "Camera Component", nullptr, biggest))
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
				if (gui::DrawMenuItem(ICON_FK_SKATE, "Rigidbody 2D Component", nullptr, biggest))
				{
					entity.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<BoxColliderComponent>() && !entity.HasComponent<CircleColliderComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_SQUARE_O, "Box Collider Component", nullptr, biggest))
				{
					entity.AddComponent<BoxColliderComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (gui::DrawMenuItem(ICON_FK_CIRCLE_O, "Circle Collider Component", nullptr, biggest))
				{
					entity.AddComponent<CircleColliderComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<SpeakerComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_VOLUME_UP, "Speaker Component", nullptr, biggest))
				{
					entity.AddComponent<SpeakerComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<ParticleSystemComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_SPINNER, "Particle System Component", nullptr, biggest))
				{
					entity.AddComponent<ParticleSystemComponent>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<NativeScriptComponent>())
			{
				if (gui::DrawMenuItem(ICON_FK_CODE, "Native Script Component", nullptr, biggest))
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
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
				ImGui::Button("##dummy", {0.0f, 0.0f});
				ImGui::PopStyleColor();
				ImGui::PopItemFlag();
				ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 156.0f);
				ImGui::Text("Local"); ImGui::SameLine(); ImGui::RadioButton("##local", &world, 0); ImGui::SameLine();
				ImGui::Text("World"); ImGui::SameLine(); ImGui::RadioButton("##world", &world, 1);

				if (!world)
				{
					gui::UISettings settings;
					bool changed = gui::DrawVec3Control("Position", tc.Position, settings, "Position of the Transform in X, Y, and Z coordinates.");
					glm::vec3 scale = { tc.Scale.x, tc.Scale.y, 1.0f };
					settings.MaxFloat = FLT_MAX;
					settings.Enabled[2] = false;
					settings.ResetValue = 1.0f;
					if (gui::DrawVec3Control("Scale", scale, settings, "Scale of the Transform along X and Y axes. Value '1' is the original size"))
					{
						tc.Scale = { scale.x, scale.y };
						changed = true;
					}

					settings.MinFloat = -180.0;
					settings.MaxFloat = 180.0;
					settings.Enabled = { false, false, true, false };
					settings.ResetValue = 0.0f;
					glm::vec3 rotation{ 0.0f, 0.0f, tc.Rotation };
					if (gui::DrawVec3Control("Rotation", rotation, settings, "Rotation of the Transform around the Z axis, measured in degrees."))
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
					bool changed = gui::DrawVec3Control("Position", pos, settings, "Position of the Transform in X, Y, and Z coordinates.");
					settings.MaxFloat = FLT_MAX;
					settings.Enabled[2] = false;
					settings.ResetValue = 1.0f;
					if (gui::DrawVec3Control("Scale", scale, settings, "Scale of the Transform along X and Y axes. Value '1' is the original size"))
						changed = true;
					settings.MinFloat = -180.0;
					settings.MaxFloat = 180.0;
					settings.Enabled = { false, false, true, false };
					settings.ResetValue = 0.0f;
					rotation = glm::degrees(rotation);
					if (gui::DrawVec3Control("Rotation", rotation, settings, "Rotation of the Transform around the Z axis, measured in degrees."))
						changed = true;

					if (changed)
					{
						transformation = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), scale);
						const auto& rel = entity.GetComponent<RelationshipComponent>();
						if (rel.Parent == entt::null)
						{
							tc.Position = pos;
							tc.Scale = { scale.x, scale.y };
							tc.Rotation = rotation.z;
						}
						else
						{
							Entity parent = { rel.Parent, internal::GetContext()->ActiveScene };
							const glm::mat4& ptransform = parent.GetComponent<TransformationComponent>();
							const glm::mat4 local = glm::inverse(ptransform) * transformation.Transform;
							math::DecomposeTransform(local, pos, scale, rotation);
							tc.Position = pos;
							tc.Scale = { scale.x, scale.y };
							tc.Rotation = glm::degrees(rotation.z);
						}
						internal::GetContext()->ActiveScene->UpdateTransform(entity);
					}
				}
			});
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			gui::DrawComponent<SpriteRendererComponent>(ICON_FK_PICTURE_O, "Sprite Renderer Component", entity, [](auto& sprite) {
				gui::UISettings settings;
				uuid id = sprite.Texture->ID;
				if (gui::DrawAssetControl("Texture", id, ".gtimg", settings,"Define which Sprite texture the component should render. Click the small dot to the right to\nopen the object picker window, and select from the list of available Sprite Assets or drag \nand drop them here from the Assets window."))
					sprite.Texture = internal::GetContext()->AssetManager.RequestAsset(id);
				settings.ResetValue = 1.0f;
				gui::DrawColorPicker("Tint Color", sprite.Color, settings, "Define the vertex color of the Sprite, which tints or recolors the Sprite's image.\nUse the color picker to set the vertex color of the rendered Sprite texture.");
				if (sprite.Texture->Type == AssetType::TEXTURE)
				{
					GPU::Texture* texture = (GPU::Texture*)sprite.Texture->Data;
					settings.MinFloat = 1.0f;
					settings.MaxFloat = FLT_MAX;
					gui::DrawFloatControl("Tiling Factor", sprite.TilingFactor, settings, "Define the number of times the chosen Sprite texture tiles in the entity.\nE.g. Tiling Factor of 3 fills the Entity with 3x3 instances of the chosen Sprite texture");
					gui::DrawVec2BoolControl("Flip", sprite.FlipX, sprite.FlipY, settings, "Flips the Sprite texture along the checked axis. This does not flip the Transform position of the Entity");
				}
				gui::DrawBoolControl("Visible", sprite.Visible, settings, "If checked the component will be rendered.");
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

		if (entity.HasComponent<AnimationComponent>())
		{
			gui::DrawComponent<AnimationComponent>(ICON_FK_FILM, "Animation Component", entity, [](auto& ac) {
				gui::UISettings settings;
				gui::DrawAssetControl("Animation Clip", ac.Animation->ID, ".gtanimation", settings, "Define which animation will be played. Click the small dot to the right to\nopen the object picker window, and select from the list of available Animation Assets or drag \nand drop them here from the Assets window.");
			});
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			gui::DrawComponent<CircleRendererComponent>(ICON_FK_CIRCLE, "Circle Renderer Component", entity, [](auto& circle) {
				gui::UISettings settings;
				settings.ResetValue = 1.0f;
				gui::DrawColorPicker("Color", circle.Color, settings,"Use the color picker to set the vertex color of the rendered Circlular Component.");
				settings.MaxFloat = 1.0f;
				gui::DrawFloatControl("Thickness", circle.Thickness, settings, "Define the area percentage of the Circlular Component to render.E.g. Value of '0.7' will\nrender 70% of the area starting from the perimeter. Max value: 1.0");
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Fade", circle.Fade, settings,"The color faints with bigger values.");
				gui::DrawBoolControl("Visible", circle.Visible, settings, "If checked the component will be rendered.");
			});
		}

		if (entity.HasComponent<TextRendererComponent>())
		{
			gui::DrawComponent<TextRendererComponent>(ICON_FK_FONT, "Text Renderer Component", entity, [](auto& tc) {
				gui::UISettings settings;
				settings.Length = 64;
				gui::DrawMultilineStringControl("Text String", tc.Text, settings, "Enter text that will be drawn on the screen (Maximum size 1 MiB).");
				settings.ResetValue = 1.0f;
				gui::DrawColorPicker("Color", tc.Color, settings, "Use the color picker to set the vertex color of the rendered Circlular Component.");
				gte::uuid id = tc.Font->ID;
				if (gui::DrawAssetControl("Font", id, ".gtfont", settings, "Define which Font the component should render. Click the small dot to the right to\nopen the object picker window, and select from the list of available Sprite Assets or drag \nand drop them here from the Assets window."))
					tc.Font = internal::GetContext()->AssetManager.RequestAsset(id);
				settings.MinUint = 1; 
				settings.MaxUint = std::numeric_limits<uint32>::max();
				gui::DrawUint32Control("Font Size", tc.Size, settings, "Desired font size used for calculating screen pixel distance.\nDoesn't reload font or changes the size of text.");
				gui::DrawBoolControl("Visible", tc.Visible, settings, "If checked the component will be rendered.");
			});
		}

		if (entity.HasComponent<CameraComponent>())
		{
			auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
			gui::DrawComponent<CameraComponent>(ICON_FK_CAMERA_RETRO, "Camera Component", entity, [&](auto& cam) {
				gui::UISettings settings;
				settings.MinFloat = 0.25f;
				settings.MaxFloat = FLT_MAX;
				bool changed = gui::DrawFloatControl("Zoom Level", ortho.ZoomLevel, settings,"Zoom out with bigger values.");
				settings.MinFloat = 0.5f;
				if (gui::DrawFloatControl("Vertical Boundary", ortho.VerticalBoundary, settings, "The top and bottom padding of the camera"))
					changed = true;
				gui::DrawBoolControl("Primary", cam.Primary, settings, "If checked the camera becomes primary");
				if (gui::DrawBoolControl("Fixed Asp. Ratio", cam.FixedAspectRatio, settings, "If checked the Aspect Ratio of the camera stays constant") && !cam.FixedAspectRatio)
					cam.AspectRatio = static_cast<float>(internal::GetContext()->ViewportSize.x) / static_cast<float>(internal::GetContext()->ViewportSize.y);
				settings.MinFloat = 0.1f;
				settings.MaxFloat = FLT_MAX;
				if (cam.FixedAspectRatio)
				{
					if (gui::DrawFloatControl("Aspect Ratio", cam.AspectRatio, settings, "Define the Aspect Ratio of the camera."))
						changed = true;
				}

				ImGui::SetCursorPosX(0.0f);
				if (ImGui::TreeNodeEx("Audio Listener", ImGuiTreeNodeFlags_OpenOnArrow))
				{
					static constexpr char* typestr[] = { "None", "Inverse", "Inverse Clamp", "Linear", "Linear Clamp", "Exponent", "Exponent Clamp" };
					int32 index = static_cast<int32>(cam.Model);
					if (gui::DrawComboControl("Distance Model", index, typestr, 7, settings,"Select Inverse, Linear or Exponent sound models"))
						cam.Model = static_cast<DistanceModel>(index);
					settings.MinFloat = 0.0f;
					settings.MaxFloat = 1.0f;
					gui::DrawFloatControl("Master Volume", cam.MasterVolume, settings,"Define the Volume of the Listener");
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

				settings.ResetValue = 1.0f;
				gui::DrawColorPicker("ColorBegin", psc.Props.ColorBegin, settings, "Define the color that the particle will start with\nUse the color picker to set the color of the particle.");
				gui::DrawColorPicker("ColorEnd", psc.Props.ColorEnd, settings, "Define the color that the particle will end with\nUse the color picker to set the color of the particle.");

				settings.ResetValue = 0.0f;
				gui::DrawVec2Control("Position", psc.Props.Position, settings, "Specify the coordinates of where particles will spawn in local 2D space.");
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawVec2Control("Size Begin", psc.Props.SizeBegin, settings, "Specify the local size that the particle will start with.");
				gui::DrawVec2Control("Size End", psc.Props.SizeEnd, settings, "Specify the local size that the particle will end with.");
				settings.MaxFloat = 0.0f;
				gui::DrawVec2Control("Velocity", psc.Props.Velocity, settings, "Specify the basic velocity that the particle will spawn with.");
				gui::DrawVec2Control("Velocity Variation", psc.Props.VelocityVariation, settings, "Specify a value that will variate the basic Velocity with which the particle will spawn.\nFor example a Velocity Variation of 1.0f will add a value in range [-0.5f, 0.5f] to the Velocity.");
				settings.MinFloat = -180.0f;
				settings.MaxFloat = 180.0f;
				gui::DrawFloatControl("Rotation", psc.Props.Rotation, settings, "Specify the starting rotation that the particle will spawn with.");
				gui::DrawFloatControl("Ang. Velocity", psc.Props.AngularVelocity, settings, "Specify the basic Angular Velocity that the particle will spawn with.");
				settings.MinFloat = 0.0f;
				gui::DrawFloatControl("Ang. Vel. Variation", psc.Props.AngularVelocityVariation, settings, "Specify a value that will variate the basic Angular Velocity with which the particle spawn.\nFor example a Angular Velocity Variation of 1.0f will add a value in range [-0.5f, 0.5f] ot the Angular Velocity.");
				gui::DrawFloatControl("Duration", psc.Props.Duration, settings, "Specify the number of seconds that the Particle System will be spawning new particles.");
				gui::DrawFloatControl("Life Time", psc.Props.LifeTime, settings, "Specify the number of seconds that a Particle should live for.");
				gui::DrawFloatControl("Emition Rate", psc.Props.EmitionRate, settings, "Specify the number of seconds that will take between spawning a new particle.");
				settings.MinUint = 0;
				settings.MaxUint = std::numeric_limits<uint32>::max();
				gui::DrawUint32Control("Max Particles", psc.Props.MaxParticles, settings, "Specify the maximum number of particles that the System can hold.\nIf this number is reached while they are particles alive the the new particle spawned will replace the oldest one.");
				gui::DrawBoolControl("Looping", psc.Props.Looping, settings, "If checked the duration parameter is being ingored and the particle system will not stop emmiting new particle ever.");
				gui::DrawBoolControl("Play On Start", psc.PlayOnStart, settings, "If checked starts emmiting particles once the Entity spawns.");
			});
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			gui::DrawComponent<Rigidbody2DComponent>(ICON_FK_SKATE, "Rigidbody 2D Component", entity, [](auto& rb) {
				static constexpr char* typestr[3] = { "Static", "Dynamic", "Kinematic" };
				gui::UISettings settings;
				int32 index = static_cast<int32>(rb.Type);
				if (gui::DrawComboControl("Body Type", index, typestr, 3, settings,"Set the RigidBody 2D's component settings, so that you can manipulate movement\n(position and rotation) behavior and Collider 2D interaction."))
					rb.Type = static_cast<BodyType>(index);
				gui::DrawVec2Control("Velocity", rb.Velocity, settings,"Set the starting velocity affecting the positional movement");
				gui::DrawFloatControl("Ang. Velocity", rb.AngularVelocity, settings,"Set the angular velocity affecting the rotational movement");
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Gravity Factor", rb.GravityFactor, settings, "Define the degree to which the Entity is affected by gravity.");
				gui::DrawBoolControl("Fixed Rotation", rb.FixedRotation, settings, "If checked the Entity never rotates");
				gui::DrawBoolControl("Bullet", rb.Bullet, settings, "If checked the Entity has Bullet-like behavior");
			});
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			gui::DrawComponent<BoxColliderComponent>(ICON_FK_SQUARE_O, "Box Collider Component", entity, [](auto& bc) {
				gui::UISettings settings;
				gui::DrawVec2Control("Offset", bc.Offset, settings, "Set the local offset of the Collider 2D geometry.");
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawVec2Control("Size", bc.Size, settings, "Set the size of the Box Collider in local space units.");
				gui::DrawFloatControl("Density", bc.Density, settings, "Change the density to change the mass calculations of the Entity's associated Rigidbody 2D");
				gui::DrawFloatControl("Friction", bc.Friction, settings, "The higher the friction the less the object slides.");
				gui::DrawFloatControl("Restitution", bc.Restitution, settings, "The higher the restitution the more the Box will bounce on contact.");
				gui::DrawFloatControl("Rest. Threshold", bc.RestitutionThreshold, settings, "The threshold of the bounciness of the Box");
				gui::DrawBoolControl("Sensor", bc.Sensor, settings, "Check this box if you want the Box Collider 2D to behave as a sensor.");
			});
		}

		if (entity.HasComponent<CircleColliderComponent>())
		{
			gui::DrawComponent<CircleColliderComponent>(ICON_FK_CIRCLE_O, "Circle Collider Component", entity, [](auto& cc) {
				gui::UISettings settings;
				gui::DrawVec2Control("Offset", cc.Offset, settings, "Set the local offset of the Collider 2D geometry.");
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Radius", cc.Radius, settings, "Set the radius of the Circle Collider in local space units.");
				gui::DrawFloatControl("Density", cc.Density, settings, "Change the density to change the mass calculations of the Entity's associated Rigidbody 2D");
				gui::DrawFloatControl("Friction", cc.Friction, settings, "The higher the friction the more the Circle will catch and roll rather than just slide.");
				gui::DrawFloatControl("Restitution", cc.Restitution, settings, "The higher the restitution the more the Circle will bounce on contact.");
				gui::DrawFloatControl("Rest. Threshold", cc.RestitutionThreshold, settings, "The threshold of the bounciness of the Circle");
				gui::DrawBoolControl("Sensor", cc.Sensor, settings, "Check this box if you want the Circle Collider to behave as a sensor.");
			});
		}

		if (entity.HasComponent<SpeakerComponent>())
		{
			gui::DrawComponent<SpeakerComponent>(ICON_FK_VOLUME_UP, "Speaker Component", entity, [](auto& speaker) {
				gui::UISettings settings;
				uuid id = speaker.AudioClip->ID;
				if (gui::DrawAssetControl("Audio Clip", id, ".gtaudio", settings, "Define which Audio Asset the component should use. Click the small dot to the right to open\nthe object picker window, and select from the list of available Audio Assets or drag and drop \nthem here from the Assets window."))
					speaker.AudioClip = internal::GetContext()->AssetManager.RequestAsset(id);
				settings.MinFloat = 0.0f;
				settings.MaxFloat = FLT_MAX;
				gui::DrawFloatControl("Volume", speaker.Volume, settings, "How loud the sound is at a distance of one world unit (one meter) from the Audio Listener.");
				gui::DrawFloatControl("Pitch", speaker.Pitch, settings, "Amount of change in pitch due to slowdown/speed up of the Audio Clip. Value 1 is normal playback speed.");
				gui::DrawFloatControl("Roll off factor", speaker.RollOffFactor, settings, "How fast the sound fades. The higher the value, the closer the Listener has to be before hearing the sound.");
				gui::DrawFloatControl("Ref. Distance", speaker.RefDistance, settings, "The distance under which the volume for the source would normally drop by half(before being influenced by rolloff factor).");
				gui::DrawFloatControl("Max Distance", speaker.MaxDistance, settings, "The distance where the sound stops attenuating at. Beyond this point it will stay at the\nvolume it would be at MaxDistance units from the listener and will not attenuate any more.");
				gui::DrawBoolControl("Looping", speaker.Looping, settings, "If checked the Audio clip loops.");
				gui::DrawBoolControl("Play On Start", speaker.PlayOnStart, settings, "If checked starts playing the sound once the Entity spawns.");
			});
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			gui::DrawComponent<NativeScriptComponent>(ICON_FK_CODE, "Native Script Component", entity, [](auto& nc) {
				gui::UISettings settings;
				uuid id = nc.ScriptAsset->ID;
				nc.ScriptAsset = internal::GetContext()->AssetManager.RequestAsset(id);
				if (gui::DrawAssetControl("Script", id, ".gtscript", settings, "Define which Script Asset the component should use. Click the small dot to the right to open\nthe object picker window, and select from the list of available Script Assets or drag and drop \nthem here from the Assets window."))
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