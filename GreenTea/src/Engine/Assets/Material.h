#pragma once

#include "Asset.h"
#ifndef GT_DIST
	#include "Image.h"
#endif
#include <Engine/Core/Ref.h>

#include <string>
#include <glm.hpp>

namespace gte {
	
	struct Material {
		std::string Name;

		Ref<Asset> Albedo = CreateRef<Asset>();
		Ref<Asset> Normal = CreateRef<Asset>();
		Ref<Asset> Metallic = CreateRef<Asset>();
		Ref<Asset> Rough = CreateRef<Asset>();
		Ref<Asset> AmbientOclussion = CreateRef<Asset>();
		Ref<Asset> Opacity = CreateRef<Asset>();
		Ref<Asset> Emission = CreateRef<Asset>();

		glm::vec4 Diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 EmitColor{ 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Metallicness = 0.33f;
		float Roughness = 0.537f;
		float Alpha = 1.0f;
		int32 IlluminationModel = 1;
		bool IsEmissive = false;
#ifndef GT_DIST
		Image img;
#endif
	};

}