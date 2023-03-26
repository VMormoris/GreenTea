#include "Systems.h"
#include "Components.h"

#include <Engine/Core/Context.h>
#include <Engine/Renderer/Renderer2D.h>

namespace gte::internal {

	void RenderScene(entt::registry* reg, const glm::mat4& eyeMatrix)
	{
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
	}

}