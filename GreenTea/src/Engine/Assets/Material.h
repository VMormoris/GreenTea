#pragma once

#include "Asset.h"

#include <Engine/Core/Ref.h>

#include <string>
#include <glm.hpp>

namespace gte {
	
	struct Material {
		std::string Name;

		Ref<Asset> Albedo;
		Ref<Asset> Normal;
		Ref<Asset> Metallic;
		Ref<Asset> AmbientOclussion;
		Ref<Asset> Opacity;
		Ref<Asset> Emission;

		glm::vec4 Diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 EmitColor{ 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Metallicness = 1.0f;
		float Shininess = 0.537f;
		float Alpha = 1.0f;
		int32 IlluminationModel = 1;
		bool IsEmissive = false;
	};

}