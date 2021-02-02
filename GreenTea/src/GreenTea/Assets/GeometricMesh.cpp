#include "GeometricMesh.h"
#include "OBJLoader.h"

#include "AssetManager.h"

namespace GTE {


	Material* GeometricMesh::FindMaterial(const std::string& str)
	{
		const std::string comparable = str.empty() ? "default" : str;
		for (uint32 i = 0; i < m_Materials.size(); i++)
		{
			if (m_Materials[i].Name == comparable) return &m_Materials[i];
		}
		return NULL;
	}

	int32 GeometricMesh::FindMaterialID(const std::string& str)
	{
		const std::string comparable = str.empty() ? "default" : str;
		for (int32 i = 0; i < m_Materials.size(); i++)
		{
			if (m_Materials[i].Name == comparable)
				return i;
		}
		return -1;
	}

	GeometricMesh::GeometricMesh(const char* filepath) { Load(filepath); }
	void GeometricMesh::Load(const char* filepath) { OBJLoader{ this }.Load(filepath); }

	void* GeometricMesh::GetPositions(void) { return &m_Vertices.Positions[0]; }
	const void* GeometricMesh::GetPositions(void) const { return &m_Vertices.Positions[0]; }
	size_t GeometricMesh::GetPositionBufferSize(void) const { return m_Vertices.Positions.size() * sizeof(glm::vec3); }

	void* GeometricMesh::GetNormals(void) { return &m_Vertices.Normals[0]; }
	const void* GeometricMesh::GetNormals(void) const { return &m_Vertices.Normals[0]; }
	size_t GeometricMesh::GetNormalBufferSize(void) const { return m_Vertices.Normals.size() * sizeof(glm::vec3); }

	void* GeometricMesh::GetTextCoords(void) { return &m_Vertices.TextCoords[0]; }
	const void* GeometricMesh::GetTextCoords(void) const { return &m_Vertices.TextCoords[0]; }
	size_t GeometricMesh::GetTextCoordBufferSize(void) const { return m_Vertices.TextCoords.size() * sizeof(glm::vec2); }

	void* GeometricMesh::GetTangents(void) { return &m_Vertices.Tangents[0]; }
	const void* GeometricMesh::GetTangents(void) const { return &m_Vertices.Tangents[0]; }
	size_t GeometricMesh::GetTangentsBufferSize(void) const { return m_Vertices.Tangents.size() * sizeof(glm::vec3); }

	void* GeometricMesh::GetBitangents(void) { return &m_Vertices.Bitangents[0]; }
	const void* GeometricMesh::GetBitangents(void) const { return &m_Vertices.Bitangents[0]; }
	size_t GeometricMesh::GetBitangentsBufferSize(void) const { return m_Vertices.Bitangents.size() * sizeof(glm::vec3); }

	const std::vector<MeshPart>& GeometricMesh::GetObjects(void) const { return m_Objects; }
	const std::vector<Material>& GeometricMesh::GetMaterials(void) const { return m_Materials; }



}