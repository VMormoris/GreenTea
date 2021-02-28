//This file is modified version of the code
//	written in the labs of AUEB during the Course of:
//	Computer Graphics 2020 - 2021
#pragma once
#include "Material.h"

#include <vector>
#include <array>

namespace GTE {

	struct MeshPart {
		std::string Name;
		int32 MaterialID;
		uint32 Start;
		uint32 End;
	};

	class ENGINE_API GeometricMesh {
	public:

		GeometricMesh() = default;
		GeometricMesh(const char* filepath);

		Material* FindMaterial(const std::string& str);
		int32 FindMaterialID(const std::string& str);

		struct Vertices {
			std::vector<glm::vec3> Positions;
			std::vector<glm::vec3> Normals;
			std::vector<glm::vec2> TextCoords;
			std::vector<glm::vec3> Tangents;
			std::vector<glm::vec3> Bitangents;
		};

		void Load(const char* filepath);

		void* GetPositions(void);
		const void* GetPositions(void) const;
		size_t GetPositionBufferSize(void) const;

		void* GetNormals(void);
		const void* GetNormals(void) const;
		size_t GetNormalBufferSize(void) const;
		
		void* GetTextCoords(void);
		const void* GetTextCoords(void) const;
		size_t GetTextCoordBufferSize(void) const;

		void* GetTangents(void);
		const void* GetTangents(void) const;
		size_t GetTangentsBufferSize(void) const;

		void* GetBitangents(void);
		const void* GetBitangents(void) const;
		size_t GetBitangentsBufferSize(void) const;

		const std::vector<MeshPart>& GetObjects(void) const;
		const std::vector<Material>& GetMaterials(void) const;

		std::array<glm::vec3, 2>& GetABB(void);
		const std::array<glm::vec3, 2>& GetABB(void) const;

	private:

		std::vector<MeshPart> m_Objects;
		std::vector<Material> m_Materials;
		Vertices m_Vertices;
		std::array<glm::vec3, 2> m_ABB;
		friend class OBJLoader;
	};

}