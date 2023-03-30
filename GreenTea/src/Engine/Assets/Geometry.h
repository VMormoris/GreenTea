#pragma once
#include <Engine/Core/Engine.h>

#include <glm.hpp>

namespace gte {

	struct ENGINE_API MeshPart {
		int32 MaterialIndex = -1;
		uint32 Start = 0;
		uint32 End = 0;

		//std::array<glm::vec3, 2> AAB = { glm::vec3(999999.0f), glm::vec3(-999999.0f) };

		MeshPart(void) = default;
		MeshPart(uint32 start, uint32 end, int32 index)
			: Start(start), End(end), MaterialIndex(index) {}
	};

	struct ENGINE_API Geometry {

		std::vector<glm::vec3> Vertices;
		std::vector<glm::vec3> Normals;
		std::vector<glm::vec2> UVs;
		std::vector<glm::vec3> Tangents;
		std::vector<glm::vec3> Bitangents;

		std::vector<MeshPart> Parts;

		struct Material {
			std::string Name;

			std::string Albedo;
			std::string Normal;
			std::string Metallic;
			std::string AmbientOclussion;
			std::string Opacity;
			std::string Emission;

			glm::vec4 Diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 EmitColor{ 0.0f, 0.0f, 0.0f, 0.0f };
			glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
			float Metallicness = 1.0f;
			float Shininess = 0.537f;
			float Alpha = 1.0f;
			int32 IlluminationModel = 1;
			bool IsEmissive = false;
		};

		std::vector<Material> Materials;

		int32 FindMaterial(const std::string& name);

		glm::vec3 ABB[2];
	};
}