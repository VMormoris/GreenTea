//This file is modified version of the code
//	written in the labs of AUEB during the Course of:
//	Computer Graphics 2020 - 2021
#pragma once
#include "Asset.h"

#include <GreenTea/Core/Ref.h>

#include <glm.hpp>
#include <string>

namespace GTE {
	struct ENGINE_API Material {

		Material();

		std::string Name;
		glm::vec4 Ambient{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 Diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 Specular{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Alpha = 1.0f;
		float Shininess = 0.0f;

		int32 IlluminationModel = 0;

		std::string DiffuseName;
		std::string SpecularName;
		std::string AmbientName;
		std::string NormalName;
		std::string BumpName;
		std::string SpecularityName;
		std::string OpacityName;

		Ref<Asset> DiffuseTexture;
		Ref<Asset> SpecularTexture;
		Ref<Asset> AmbientTexture;
		Ref<Asset> NormalTexture;
		Ref<Asset> BumpTexture;
		Ref<Asset> SpecularityTexture;
		Ref<Asset> OpacityTexture;
	};
}