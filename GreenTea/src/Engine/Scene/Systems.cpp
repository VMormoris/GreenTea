#include "Systems.h"
#include "Components.h"

#include <Engine/Core/Context.h>
#include <Engine/Renderer/Renderer2D.h>
#include <Engine/Renderer/Renderer3D.h>

#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

static float sAccumulator = 0.0f;

namespace gte::internal {

	void RenderScene(entt::registry* reg, const glm::mat4& eyeMatrix)
	{
		let meshID = uuid{ "34D15D68-F7CF-4275-8DDE-F8F01F0B7361" };
		Ref<Asset> asset = internal::GetContext()->AssetManager.RequestAsset(meshID);
		if (asset->Type != AssetType::MESH) return;

		SceneData data;		
		auto view = reg->view<TransformComponent, CameraComponent>(entt::exclude<RelationshipComponent>);
		for (auto&& [entityID, tc, cam] : view.each())
		{
			data.CameraPos = tc.Position;
			let orientation = glm::quat(glm::radians(tc.Rotation));
			data.CameraDir = glm::rotate(orientation, { 0.0f, 0.0f, -1.0f });
			data.EyeMatrix = eyeMatrix;
			data.ProjectionMatrix = cam.ProjectionMatrix;
			data.ViewMatrix = cam.ViewMatrix;
			data.Target = internal::GetContext()->ViewportFBO;
		}

		sAccumulator += 1.0f / 60.0f * glm::pi<float>() * 12.5f;
		let matrix = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f }) 
			* glm::toMat4(glm::quat(glm::radians(glm::vec3{0.0f, sAccumulator, 0.0f})));
		Renderer3D::BeginScene(data);
		Renderer3D::SubmitMesh(matrix, (GPU::Mesh*)asset->Data, 0);
		Renderer3D::EndScene();
		/*
		entt::insertion_sort algo;
		reg->sort<TransformationComponent>([](const auto& lhs, const auto& rhs) { return lhs.Transform[3].z < rhs.Transform[3].z; }, algo);
		
		Renderer2D::BeginScene(eyeMatrix);

		auto view = reg->view<TransformationComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, tc] : view.each())
		{
			if (auto* sprite = reg->try_get<SpriteRendererComponent>(entityID))
			{
				if (!sprite->Visible) return;
				sprite->Texture = GetContext()->AssetManager.RequestAsset(sprite->Texture->ID);
				if (sprite->Texture->Type == AssetType::TEXTURE)
				{
					TextureCoordinates coords = sprite->Coordinates;
					if (sprite->FlipX)
					{
						float x = coords.BottomLeft.x;
						coords.BottomLeft.x = coords.TopRight.x;
						coords.TopRight.x = x;
					}
					if (sprite->FlipY)
					{
						float y = coords.BottomLeft.y;
						coords.BottomLeft.x = coords.TopRight.y;
						coords.TopRight.y = y;
					}
					Renderer2D::DrawQuad(tc, (GPU::Texture*)sprite->Texture->Data, coords, (uint32)entityID, sprite->Color, sprite->TilingFactor);
				}
				else
					Renderer2D::DrawQuad(tc, (uint32)entityID, sprite->Color);
			}
			else if (auto* circle = reg->try_get<CircleRendererComponent>(entityID))
			{
				if (circle->Visible)
					Renderer2D::DrawCircle(tc, circle->Color, (uint32)entityID, circle->Thickness, circle->Fade);
			}
		}

		if (GetContext()->Playing)
		{
			auto particles = reg->view<ParticleSystemComponent, TransformationComponent>(entt::exclude<filters::Disabled>);
			particles.each([](const auto& psc, const auto& tc) { psc.System->Render(tc); });
		}

		auto texts = reg->view<TransformationComponent, TextRendererComponent>(entt::exclude<filters::Disabled>);
		for (auto&& [entityID, tc, text] : texts.each())
		{
			text.Font = GetContext()->AssetManager.RequestAsset(text.Font->ID);
			if (text.Font->Type != AssetType::TEXTURE || !text.Visible) continue;
			Ref<Asset> font = internal::GetContext()->AssetManager.RequestAsset(text.Font->ID, true);
			Renderer2D::DrawString(text.Text, tc, text.Size, (GPU::Texture*)text.Font->Data, (Font*)font->Data, text.Color);
		}

		//texts.each([](const auto& tc, const auto& text)
		//{
		//	text.Font = GetContext()->AssetManager.RequestAsset(text.Font->ID);
		//	if (text.Font->Type != AssetType::TEXTURE || !text.Visible) return;
		//	Ref<Asset> font = internal::GetContext()->AssetManager.RequestAsset(text.Font->ID, true);
		//	Renderer2D::DrawString(text.Text, tc, text.Size, (GPU::Texture*)text.Font->Data, (Font*)font->Data, text.Color);
		//});

		Renderer2D::EndScene();
		*/
	}

}