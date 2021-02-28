//This file is modified version of the code
//	written in the labs of AUEB during the Course of:
//	Computer Graphics 2020 - 2021
#pragma once
#include "GeometricMesh.h"

namespace GTE {

	/**
	* @breif Loader for the .obj Format
	* @details It supports triangles and quads (it breaks them into two triangles)
	*	It supports negative indices in the faces
	*	If there are no normals it creates them
	*	Returns a Mesh object on RAM or a List of Triangles for Path Tracing
	*	-- Thread safe and possible the new version
	*/
	class ENGINE_API OBJLoader {
	public:
		OBJLoader(GeometricMesh* mesh);
		GeometricMesh* Load(const char* filepath);

	private:

		// Read Positions as [x, y, z]
		void ReadPositions(const char* buffer);
		// Read Texture Coordinates as [u, v]
		void ReadTextCoords(const char* buffer);
		// Read Normals as [x, y, z]
		void ReadNormals(const char* buffer);
		// Format v/vt/vn v/vt/vn v/vt/vn, only read a triangle or a quad (Or more not tested)
		void ReadFaces(const char* buffer);
		// Format v/vt/vn v/vt/vn v/vt/vn, only read a triangle or a quad
		void ReadFaceLimited(const char* buffer);

		glm::ivec3 ReadFaceComponent(const char* buffer, int32& offset);
		void ReadUseMTL(const char* buffer, int32& currentMaterialID);
		void ReadMTLLib(const char* buffer, const char* filepath);
		void AddNewGroup(const char* buffer, int32& currentMaterialID);
		void ParseMTL(const char* filepath);

		void GenerateDataFromFaces();

		void CalculateFlatNormals();
		void CalculateAvgNormals(std::vector<glm::vec3>& shared_vertices, std::vector<glm::vec3>& normals, std::vector<uint32>& elements);
		void CalculateTangents(void);

		void MoveVolume(void);

	private:

		std::vector<glm::vec3> m_SharedPositions;
		std::vector<glm::vec3> m_SharedNormals;
		std::vector<glm::vec2> m_SharedTextCoords;
		
		struct Face {
			glm::ivec3 Positions;
			glm::ivec3 Normals;
			glm::ivec3 TextCoords;
		};

		std::vector<Face> m_SharedFaces;

		std::vector<uint32> m_Elements;
		bool m_HasTextures = false;
		bool m_HasNormals = false;
		
		std::string m_FolderPath;
		GeometricMesh* m_Mesh = nullptr;
		glm::vec3 m_Mins;
		glm::vec3 m_Maxes;
	};

}