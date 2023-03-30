//This file is modified version of the code
//	written in the labs of AUEB during the Course of:
//	Computer Graphics 2020 - 2021
#pragma once

#include <GreenTea.h>

class OBJLoader {
public:

	OBJLoader(void) = default;
	~OBJLoader(void);

	gte::Geometry* Load(const char* filepath);

private:

	struct Face {
		glm::ivec3 Vertices;
		glm::ivec3 Normals;
		glm::ivec3 UVs;

		Face(void) = default;
	};
private:

	void ReadVertices(const char* buff);
	void ReadUVs(const char* buff);
	void ReadNormals(const char* buff);
	void ReadFaces(const char* buff);
	void ReadFacesLimited(const char* buff);

	glm::ivec3 ReadFaceComponent(const char* buff, int32& offset);

	void ReadMTL(const char* buff, int32& materialID);
	void ReadMTLLib(const char* buff, const char* filename);
	void ParseMTL(const char* filename);

	void AddGroup(const char* buff, int32& materialID);

	void GenerateDataFromFaces(void);

	void CalculateFlatNormals(void);
	void CalculateAverageNormals(void);
	void CalculateTangents(void);

	void MoveVolume(void);

private:
	//Actual Geometry in usable form
	gte::Geometry* mMesh = nullptr;

	//Shared values as readen from file
	std::vector<glm::vec3> mVertices;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mUVs;
	std::vector<Face> mFaces;

	// the element for calculating normals if they don't exists
	std::vector<uint32> mElements;

	// Folder path (usefull for find other files that are referenced by obj)
	std::string mFolder;

	// Vectors to find abb values
	glm::vec3 mMins = glm::vec3{ 999999.0f };
	glm::vec3 mMaxes = glm::vec3{ -999999.0f };

	bool mHasTextures = false;
};