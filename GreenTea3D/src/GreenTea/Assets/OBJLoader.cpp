//This file is modified version of the code
//	written in the labs of AUEB during the Course of:
//	Computer Graphics 2020 - 2021
#include "OBJLoader.h"
#include "AssetManager.h"

#include <GreenTea/Core/utils.h>
#include <GreenTea/Core/Logger.h>

#include <fstream>

namespace GTE {

	OBJLoader::OBJLoader(GeometricMesh* mesh)
		: m_Mesh(mesh) {}

	GeometricMesh* OBJLoader::Load(const char* filepath)
	{
		m_SharedPositions.clear();
		m_SharedNormals.clear();
		m_SharedTextCoords.clear();
		m_Elements.clear();
		m_SharedFaces.clear();

		m_HasTextures = m_HasNormals = false;
		m_Mins = glm::vec3(999999.0f);
		m_Maxes = glm::vec3(-999999.0f);

		char buffer[1024];
		char str[1024];
		
		const std::string Filepath(filepath);
		m_FolderPath = utils::get_path(Filepath);
		
		FILE* pFile;
		pFile = fopen(filepath, "r");
		ENGINE_ASSERT(pFile != NULL, "Error while opening file: ", filepath);
		
		int32 str_pos;
		
		//add a default Material
		//m_Mesh->m_Materials.push_back(Material());

		//add a default MeshObject
		MeshPart DefaultObj;
		DefaultObj.Start = 0;
		DefaultObj.MaterialID = 0;
		m_Mesh->m_Objects.push_back(DefaultObj);
		int32 currentMaterialID = 0;


		//read the file
		while (fgets(buffer, 1024, pFile) != NULL)
		{
			if (sscanf(buffer, "%s %n", str, &str_pos) >= 1)
			{
				if (strcmp(str, "v") == 0) ReadPositions(buffer + str_pos);
				else if (strcmp(str, "vt") == 0) ReadTextCoords(buffer + str_pos);
				else if (strcmp(str, "vn") == 0) ReadNormals(buffer + str_pos);
				else if (strcmp(str, "f") == 0) ReadFaceLimited(buffer + str_pos);
				else if (strcmp(str, "usemtl") == 0) ReadUseMTL(buffer + str_pos, currentMaterialID);
				else if (strcmp(str, "mtllib") == 0) ReadMTLLib(buffer + str_pos, filepath);
				else if (strcmp(str, "g") == 0 || strcmp(str, "o") == 0) AddNewGroup(buffer + str_pos, currentMaterialID);
				else if (strcmp(str, "#") == 0) { /* ignoring this line */ }
				else { /* ignoring this line */ }
			}
		}
		fclose(pFile);

		//Update Vertices
		MoveVolume();

		//Generate Vertices from Faces
		GenerateDataFromFaces();

		//Close the last object
		m_Mesh->m_Objects.back().End = static_cast<uint32>(m_Mesh->m_Vertices.Positions.size());
		m_Mesh->m_Objects.erase(std::remove_if(m_Mesh->m_Objects.begin(), m_Mesh->m_Objects.end(), [](MeshPart ob) {return ob.Start == ob.End; }), m_Mesh->m_Objects.end());

		//If normal's doesn't exist create them
		if (m_Mesh->m_Vertices.Normals.empty())
		{
			CalculateAvgNormals(m_SharedPositions, m_Mesh->m_Vertices.Normals, m_Elements);
		}

		if (std::find_if(m_Mesh->m_Materials.begin(), m_Mesh->m_Materials.end(), [](const auto& mat) {return mat.BumpName.size() > 0;}) != m_Mesh->m_Materials.end() ||
			std::find_if(m_Mesh->m_Materials.begin(), m_Mesh->m_Materials.end(), [](const auto& mat) {return mat.NormalName.size() > 0; }) != m_Mesh->m_Materials.end())
		{
			CalculateTangents();
		}

		return m_Mesh;
	}

	void OBJLoader::ReadPositions(const char* buffer)
	{
		glm::vec3 v;
		char* pEnd;
		v.x = strtof(buffer, &pEnd);
		v.y = strtof(pEnd, &pEnd);
		v.z = strtof(pEnd, &pEnd);
		//sscanf(buff, "%f %f %f", &v.x, &v.y, &v.z);
		m_SharedPositions.push_back(v);

		//Update Bounding box
		if (v.x < m_Mins.x)
			m_Mins.x = v.x;
		else if (v.x > m_Maxes.x)
			m_Maxes.x = v.x;

		if (v.y < m_Mins.y)
			m_Mins.y = v.y;
		else if (v.y > m_Maxes.y)
			m_Maxes.y = v.y;

		if (v.z < m_Mins.z)
			m_Mins.z = v.z;
		else if (v.z > m_Maxes.z)
			m_Maxes.z = v.z;
	}

	void OBJLoader::MoveVolume(void)
	{
		const glm::vec3 volCenter = (m_Maxes - m_Mins) / 2.0f + m_Mins;
		for (auto& pos : m_SharedPositions)
			pos -= volCenter;
		m_Mesh->m_ABB[0] = m_Mins - volCenter;
		m_Mesh->m_ABB[1] = m_Maxes - volCenter;
	}

	inline void OBJLoader::ReadTextCoords(const char* buffer)
	{
		glm::vec2 vt;
		char* pEnd;
		vt.x = strtof(buffer, &pEnd);
		vt.y = strtof(pEnd, &pEnd);
		//sscanf(buff, "%f %f", &vt.x, &vt.y);
		m_SharedTextCoords.push_back(vt);
	}
	// read normals x,y,z
	inline void OBJLoader::ReadNormals(const char* buffer)
	{
		glm::vec3 n;
		char* pEnd;
		n.x = strtof(buffer, &pEnd);
		n.y = strtof(pEnd, &pEnd);
		n.z = strtof(pEnd, &pEnd);
		//sscanf(buff, "%f %f %f", &n.x, &n.y, &n.z);
		m_SharedNormals.push_back(n);
	}

	// format v/vt/vn v/vt/vn v/vt/vn, only read a triangle or a quad (Or more not tested)
	void OBJLoader::ReadFaces(const char* buffer)
	{
		int32 step = 0;
		std::vector<glm::ivec3> vertices_pointers;
		vertices_pointers.reserve(6);
		glm::ivec3 vertex_pointer(0);

		vertex_pointer = ReadFaceComponent(buffer + step, step);
		//while (glm::all(glm::greaterThanEqual(vertex_pointer, glm::ivec3(0))))
		while (vertex_pointer.x >= 0)// && (vertex_pointer.x >= 0 || vertex_pointer.y >= 0 || vertex_pointer.z >= 0)) // vertex index should always exist
		{
			vertices_pointers.push_back(vertex_pointer);
			vertex_pointer = ReadFaceComponent(buffer + step, step);
		}

		/*
		*  I read 5 vertices                              |1|2|3|4|5|
		*  I add the extra necessery padding ( pad = 2 )
		*  So i add 2*pad+size                            |1|2|3|4|5|_|_|_|_|
		*  Then i put from the last index to the 3rd
		*  into their correct positions                   |1|2|3|4|5|_|_|_|5|  and  |1|2|3|4|5|4|_|_|5|
		*  Then put at x and x+1 position
		*  ( e.g. from 4th -> last with i+=3 ) the
		*  correct vertices
		*  |1|2|3|x|5|4|_|_|5| -> |1|2|3|1|5|_|_|_|5| and |1|2|3|1|x|4|_|_|5| -> |1|2|3|1|3|4|_|_|5|
		*
		*/

		int32 number_of_faces = static_cast<int32>(vertices_pointers.size() - 3) + 1;

		for (int32 face = 0; face < number_of_faces; face++)
		{
			//break them using triangle fan :( it is easy for quads

			Face f;
			f.Positions = glm::ivec3(vertices_pointers[face + 0].x, vertices_pointers[face + 1].x, vertices_pointers[face + 2].x);
			f.Normals = glm::ivec3(vertices_pointers[face + 0].y, vertices_pointers[face + 1].y, vertices_pointers[face + 2].y);
			f.TextCoords = glm::ivec3(vertices_pointers[face + 0].z, vertices_pointers[face + 1].z, vertices_pointers[face + 2].z);

			m_SharedFaces.push_back(f);
		}
	}

	void OBJLoader::ReadFaceLimited(const char* buffer)
	{
		int32 step = 0;
		glm::ivec3 vertices_pointers[4];
		vertices_pointers[0] = ReadFaceComponent(buffer + step, step);
		vertices_pointers[1] = ReadFaceComponent(buffer + step, step);
		vertices_pointers[2] = ReadFaceComponent(buffer + step, step);
		vertices_pointers[3] = ReadFaceComponent(buffer + step, step);

		// if it is a triangle
		if (vertices_pointers[0].x >= 0 && vertices_pointers[1].x >= 0 && vertices_pointers[2].x >= 0)
		{
			Face f;
			f.Positions = glm::ivec3(vertices_pointers[0].x, vertices_pointers[1].x, vertices_pointers[2].x);
			f.Normals = glm::ivec3(vertices_pointers[0].y, vertices_pointers[1].y, vertices_pointers[2].y);
			f.TextCoords = glm::ivec3(vertices_pointers[0].z, vertices_pointers[1].z, vertices_pointers[2].z);
			m_SharedFaces.push_back(f);

			// if it was a quad, add another triangle so we can form a quad
			if (vertices_pointers[3].x >= 0)
			{
				Face f;
				f.Positions = glm::ivec3(vertices_pointers[2].x, vertices_pointers[3].x, vertices_pointers[0].x);
				f.Normals = glm::ivec3(vertices_pointers[2].y, vertices_pointers[3].y, vertices_pointers[0].y);
				f.TextCoords = glm::ivec3(vertices_pointers[2].z, vertices_pointers[3].z, vertices_pointers[0].z);
				m_SharedFaces.push_back(f);
			}
		}
	}

	glm::ivec3 OBJLoader::ReadFaceComponent(const char* buffer, int& offset)
	{
		int32 index = 0;
		int32 v, vt, vn;
		// v/vt/vn format
		if (sscanf(buffer, "%d/%d/%d%n", &v, &vt, &vn, &index) >= 3)
		{
			v += v < 0 ? (int32)m_SharedPositions.size() : -1;
			vn += vn < 0 ? (int32)m_SharedNormals.size() : -1;
			vt += vt < 0 ? (int32)m_SharedTextCoords.size() : -1;
			m_Elements.push_back(v);
			offset += index;
			return glm::ivec3(v, vn, vt);
		}
		// v//vn format
		if (sscanf(buffer, "%d//%d%n", &v, &vn, &index) >= 2)
		{
			v += v < 0 ? (int32)m_SharedPositions.size() : -1;
			vn += vn < 0 ? (int32)m_SharedNormals.size() : -1;
			m_Elements.push_back(v);
			offset += index;
			return glm::ivec3(v, vn, -1);
		}
		// v/vt format
		if (sscanf(buffer, "%d/%d%n", &v, &vt, &index) >= 2)
		{
			v += v < 0 ? (int32)m_SharedPositions.size() : -1;
			vt += vt < 0 ? (int32)m_SharedTextCoords.size() : -1;
			m_Elements.push_back(v);
			offset += index;
			return glm::ivec3(v, -1, vt);
		}
		// v format
		if (sscanf(buffer, "%d%n", &v, &index) >= 1)
		{
			v += v < 0 ? (int32)m_SharedPositions.size() : -1;
			m_Elements.push_back(v);
			offset += index;
			return glm::ivec3(v, -1, -1);
		}
		return glm::ivec3(-1);
	}

	void OBJLoader::GenerateDataFromFaces()
	{
		m_HasTextures = !m_SharedTextCoords.empty();

		m_Mesh->m_Vertices.Positions.reserve(m_SharedFaces.size() * 3);
		m_Mesh->m_Vertices.Normals.reserve(m_SharedFaces.size() * 3);
		if (m_HasTextures)
			m_Mesh->m_Vertices.TextCoords.reserve(m_SharedFaces.size() * 3);

		const glm::ivec3 zeroVec(0);

		for (int32 face = 0; face < m_SharedFaces.size(); face++)
		{
			m_Mesh->m_Vertices.Positions.push_back(m_SharedPositions[m_SharedFaces[face].Positions.x]);
			m_Mesh->m_Vertices.Positions.push_back(m_SharedPositions[m_SharedFaces[face].Positions.y]);
			m_Mesh->m_Vertices.Positions.push_back(m_SharedPositions[m_SharedFaces[face].Positions.z]);

			if (glm::any(glm::greaterThanEqual(m_SharedFaces[face].Normals, glm::ivec3(0))))
			{
				m_Mesh->m_Vertices.Normals.push_back(m_SharedNormals[m_SharedFaces[face].Normals.x]);
				m_Mesh->m_Vertices.Normals.push_back(m_SharedNormals[m_SharedFaces[face].Normals.y]);
				m_Mesh->m_Vertices.Normals.push_back(m_SharedNormals[m_SharedFaces[face].Normals.z]);

				if (glm::any(glm::greaterThanEqual(m_SharedFaces[face].TextCoords, glm::ivec3(0))))
				{
					m_Mesh->m_Vertices.TextCoords.push_back(m_SharedTextCoords[m_SharedFaces[face].TextCoords.x]);
					m_Mesh->m_Vertices.TextCoords.push_back(m_SharedTextCoords[m_SharedFaces[face].TextCoords.y]);
					m_Mesh->m_Vertices.TextCoords.push_back(m_SharedTextCoords[m_SharedFaces[face].TextCoords.z]);
				}
				else if (m_HasTextures)
				{
					m_Mesh->m_Vertices.TextCoords.emplace_back(0.f);
					m_Mesh->m_Vertices.TextCoords.emplace_back(0.f);
					m_Mesh->m_Vertices.TextCoords.emplace_back(0.f);
				}
			}
		}
	}

	void OBJLoader::CalculateFlatNormals()
	{
		for (uint32 i = 0; i < m_Mesh->m_Vertices.Positions.size(); i++)
		{
			if ((i % 3) == 2) {
				glm::vec3 normal = glm::normalize(glm::cross(
					m_Mesh->m_Vertices.Positions[i] - m_Mesh->m_Vertices.Positions[i - 2],
					m_Mesh->m_Vertices.Positions[i - 1] - m_Mesh->m_Vertices.Positions[i - 2]));
				for (int32 n = 0; n < 3; n++)
					m_Mesh->m_Vertices.Normals.push_back(normal);
			}
		}
	}

	void OBJLoader::CalculateAvgNormals(std::vector<glm::vec3>& shared_vertices, std::vector<glm::vec3>& normals, std::vector<uint32>& elements)
	{
		std::vector<int32> nb_seen;
		std::vector<glm::vec3> temp_normals;
		temp_normals.resize(shared_vertices.size(), glm::vec3(0.0, 0.0, 0.0));
		nb_seen.resize(shared_vertices.size(), 0);
		for (uint32 i = 0; i < elements.size(); i += 3) {
			uint32 ia = elements[i];
			uint32 ib = elements[i + 1];
			uint32 ic = elements[i + 2];
			glm::vec3 normal = glm::normalize(glm::cross(
				glm::vec3(shared_vertices[ib]) - glm::vec3(shared_vertices[ia]),
				glm::vec3(shared_vertices[ic]) - glm::vec3(shared_vertices[ia])));

			int32 v[3];  v[0] = ia;  v[1] = ib;  v[2] = ic;
			for (int32 j = 0; j < 3; j++) {
				uint32 cur_v = v[j];
				nb_seen[cur_v]++;
				if (nb_seen[cur_v] == 1) {
					temp_normals[cur_v] = normal;
				}
				else {
					// average
					temp_normals[cur_v].x = temp_normals[cur_v].x * (1.0f - 1.0f / nb_seen[cur_v]) + normal.x * 1.0f / nb_seen[cur_v];
					temp_normals[cur_v].y = temp_normals[cur_v].y * (1.0f - 1.0f / nb_seen[cur_v]) + normal.y * 1.0f / nb_seen[cur_v];
					temp_normals[cur_v].z = temp_normals[cur_v].z * (1.0f - 1.0f / nb_seen[cur_v]) + normal.z * 1.0f / nb_seen[cur_v];
					temp_normals[cur_v] = glm::normalize(temp_normals[cur_v]);
				}
			}
		}
		// add normals
		for (uint32 i = 0; i < elements.size(); i += 3)
		{
			uint32 ia = elements[i];
			uint32 ib = elements[i + 1];
			uint32 ic = elements[i + 2];
			normals.push_back(temp_normals[ia]);
			normals.push_back(temp_normals[ib]);
			normals.push_back(temp_normals[ic]);
		}
	}

	void OBJLoader::CalculateTangents(void)
	{
		m_Mesh->m_Vertices.Tangents.clear();
		for (uint32 i = 0; i < m_Mesh->m_Vertices.Positions.size(); i += 3)
		{
			glm::vec3& v0 = m_Mesh->m_Vertices.Positions[i + 0];
			glm::vec3& v1 = m_Mesh->m_Vertices.Positions[i + 1];
			glm::vec3& v2 = m_Mesh->m_Vertices.Positions[i + 2];

			glm::vec2& uv0 = m_Mesh->m_Vertices.TextCoords[i + 0];
			glm::vec2& uv1 = m_Mesh->m_Vertices.TextCoords[i + 1];
			glm::vec2& uv2 = m_Mesh->m_Vertices.TextCoords[i + 2];

			// edges of the triangle : position delta
			glm::vec3 deltaPos1 = v1 - v0;
			glm::vec3 deltaPos2 = v2 - v0;

			// uv delta
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;
			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 b = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			// set the same tangent for all the three vertices of the triangle.
			// they will be merged later
			m_Mesh->m_Vertices.Tangents.push_back(tangent);
			m_Mesh->m_Vertices.Tangents.push_back(tangent);
			m_Mesh->m_Vertices.Tangents.push_back(tangent);

			m_Mesh->m_Vertices.Bitangents.push_back(b);
			m_Mesh->m_Vertices.Bitangents.push_back(b);
			m_Mesh->m_Vertices.Bitangents.push_back(b);
		}

		for (uint32 i = 0; i < m_Mesh->m_Vertices.Positions.size(); i += 1)
		{
			glm::vec3& n = m_Mesh->m_Vertices.Normals[i];
			glm::vec3& t = m_Mesh->m_Vertices.Tangents[i];
			glm::vec3& b = m_Mesh->m_Vertices.Bitangents[i];

			t = glm::normalize(t - n * glm::dot(n, t));
			if (glm::dot(glm::cross(n, t), b) < 0.0f)
				t = t * -1.0f;
		}

	}

	void OBJLoader::ReadUseMTL(const char* buffer, int32& currentMaterialID)
	{
		char temp[1024];
		sscanf(buffer, "%s", temp);
		// read the material
		std::string str(temp);

		//check if we have already defined a material
		if (m_Mesh->m_Objects.back().MaterialID > 0)
		{
			// set where the current object ends
			m_Mesh->m_Objects.back().End = 3 * static_cast<uint32>(m_SharedFaces.size());
			//create a new MeshObject
			MeshPart mo;
			mo.Name = m_Mesh->m_Objects.back().Name;
			mo.MaterialID = m_Mesh->FindMaterialID(str);
			mo.Start = 3 * static_cast<uint32>(m_SharedFaces.size());
			m_Mesh->m_Objects.push_back(mo);
		}
		else
		{
			m_Mesh->m_Objects.back().MaterialID = m_Mesh->FindMaterialID(str);
		}
		currentMaterialID = m_Mesh->m_Objects.back().MaterialID;
	}

	void OBJLoader::ReadMTLLib(const char* buffer, const char* filename)
	{
		char temp[1024];
		sscanf(buffer, "%s", temp);

		std::string str(temp);
		std::string folder(filename);
		folder = m_FolderPath;
		str = folder + str;
		ParseMTL(str.c_str());
	}

	void OBJLoader::AddNewGroup(const char* buffer, int32& currentMaterialID)
	{
		char name[1024];
		sscanf(buffer, "%s", name);

		// end the previous MeshObject
		m_Mesh->m_Objects.back().End = 3 * static_cast<uint32>(m_SharedFaces.size());

		// create a new object
		MeshPart mo;
		mo.Start = 3 * static_cast<uint32>(m_SharedFaces.size());
		mo.MaterialID = currentMaterialID;
		mo.Name = name;
		m_Mesh->m_Objects.push_back(mo);
	}

	void OBJLoader::ParseMTL(const char* filepath)
	{
		std::ifstream in(filepath, std::ios::in);
		ENGINE_ASSERT(in, "Cannot open material: ", filepath);

		std::string Filepath(filepath);
		std::string folder = utils::get_path(Filepath);

		//the materials to return
		Material temp;
		temp.Name = "default";
		m_Mesh->m_Materials.push_back(temp);
		int32 currentMaterialID = static_cast<int32>(m_Mesh->m_Materials.size()) - 1;
		const int32 firsMaterialID = currentMaterialID;

		float r, g, b;

		std::string line;
		while (getline(in, line))
		{
			if (line.size() == 0) continue;

			{
				int32 pos = 0;
				while ((line[pos] == ' ' || line[pos] == '\n' || line[pos] == '\r' || line[pos] == '\t') && pos < line.size())
					pos++;
				line = line.substr(pos);
			}

			if (line.substr(0, 7) == "newmtl ") { // read vertices x,y,z
				std::istringstream s(line.substr(7));
				std::string str;
				s >> str;
				// if it is the default mtl
				str = (str.empty()) ? "default" : str;

				// find if the material with that name exists
				currentMaterialID = -1;
				for (uint32 i = 0; i < m_Mesh->m_Materials.size(); i++)
				{
					if (m_Mesh->m_Materials[i].Name == str)
					{
						currentMaterialID = i;
						break;
					}
				}
				if (currentMaterialID == -1) // didn't find  the material
				{
					Material mat;
					mat.Name = str;
					m_Mesh->m_Materials.push_back(mat);
					currentMaterialID = (int32)m_Mesh->m_Materials.size() - 1;
				}
				//printf("material name = %s\n",currentMat->name.c_str());
			}
			else if (line.substr(0, 3) == "Ka ") { // read ambient color
				std::istringstream s(line.substr(3));
				s >> r; s >> g; s >> b;
				//printf("ambient =%f %f %f\n",r,g,b);
				m_Mesh->m_Materials[currentMaterialID].Ambient[0] = r;
				m_Mesh->m_Materials[currentMaterialID].Ambient[1] = g;
				m_Mesh->m_Materials[currentMaterialID].Ambient[2] = b;
				m_Mesh->m_Materials[currentMaterialID].Ambient[3] = 1;
			}
			else if (line.substr(0, 3) == "Kd ") { // read diffuse color
				std::istringstream s(line.substr(3));
				s >> r; s >> g; s >> b;
				//printf("diffuse =%f %f %f\n",r,g,b);
				m_Mesh->m_Materials[currentMaterialID].Diffuse[0] = r;
				m_Mesh->m_Materials[currentMaterialID].Diffuse[1] = g;
				m_Mesh->m_Materials[currentMaterialID].Diffuse[2] = b;
				m_Mesh->m_Materials[currentMaterialID].Diffuse[3] = 1;
			}
			else if (line.substr(0, 3) == "Ks ") { // read specular color
				std::istringstream s(line.substr(3));
				s >> r; s >> g; s >> b;
				m_Mesh->m_Materials[currentMaterialID].Specular[0] = r;
				m_Mesh->m_Materials[currentMaterialID].Specular[1] = g;
				m_Mesh->m_Materials[currentMaterialID].Specular[2] = b;
				m_Mesh->m_Materials[currentMaterialID].Specular[3] = 1.0f;
			}
			else if (line.substr(0, 3) == "Ns ") { // read shineness
				std::istringstream s(line.substr(3));
				s >> r;
				m_Mesh->m_Materials[currentMaterialID].Shininess = r;
			}
			else if (line.substr(0, 2) == "d ") { // read alpha
				std::istringstream s(line.substr(2));
				s >> r;
				m_Mesh->m_Materials[currentMaterialID].Alpha = r;
			}
			else if (line.substr(0, 6) == "illum ") { // read illumination model
				std::istringstream s(line.substr(6));
				int i;
				s >> i;
				m_Mesh->m_Materials[currentMaterialID].IlluminationModel = i;
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 7), "map_kd ")) { // read texture
				std::istringstream s(line.substr(7));
				s >> m_Mesh->m_Materials[currentMaterialID].DiffuseName;
				m_Mesh->m_Materials[currentMaterialID].DiffuseName = folder + m_Mesh->m_Materials[currentMaterialID].DiffuseName;
				m_Mesh->m_Materials[currentMaterialID].DiffuseTexture = AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].DiffuseName.c_str());
				m_Mesh->m_Materials[currentMaterialID].Diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 13), "map_emissive ")) {// read ambient texture
				std::istringstream s(line.substr(13));
				s >> m_Mesh->m_Materials[currentMaterialID].AmbientName;
				m_Mesh->m_Materials[currentMaterialID].AmbientName = folder + m_Mesh->m_Materials[currentMaterialID].AmbientName;
				m_Mesh->m_Materials[currentMaterialID].AmbientTexture = AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].AmbientName.c_str());
				m_Mesh->m_Materials[currentMaterialID].Ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 7), "map_ka ")) { // read ambient texture
				std::istringstream s(line.substr(7));
				s >> m_Mesh->m_Materials[currentMaterialID].AmbientName;
				m_Mesh->m_Materials[currentMaterialID].AmbientName = folder + m_Mesh->m_Materials[currentMaterialID].AmbientName;
				m_Mesh->m_Materials[currentMaterialID].AmbientTexture = AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].AmbientName.c_str());
				m_Mesh->m_Materials[currentMaterialID].Ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 7), "map_ks ")) { // read specular texture
				std::istringstream s(line.substr(7));
				s >> m_Mesh->m_Materials[currentMaterialID].SpecularName;
				m_Mesh->m_Materials[currentMaterialID].SpecularName = folder + m_Mesh->m_Materials[currentMaterialID].SpecularName;
				m_Mesh->m_Materials[currentMaterialID].SpecularTexture = AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].SpecularName.c_str());
				m_Mesh->m_Materials[currentMaterialID].Specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				m_Mesh->m_Materials[currentMaterialID].Shininess = 0.0f;
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 6), "map_d ")) { // read opacity texture
				std::istringstream s(line.substr(6));
				s >> m_Mesh->m_Materials[currentMaterialID].OpacityName;
				m_Mesh->m_Materials[currentMaterialID].OpacityName = folder + m_Mesh->m_Materials[currentMaterialID].OpacityName;
				m_Mesh->m_Materials[currentMaterialID].OpacityTexture = AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].OpacityName.c_str());		
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 9), "map_bump ")) { // read bump texture (NORMAL)
				std::istringstream s(line.substr(9));
				s >> m_Mesh->m_Materials[currentMaterialID].NormalName;
				m_Mesh->m_Materials[currentMaterialID].NormalName = folder + m_Mesh->m_Materials[currentMaterialID].NormalName;
				m_Mesh->m_Materials[currentMaterialID].NormalTexture = AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].NormalName.c_str());
			}
			else if (utils::compareStringIgnoreCase(line.substr(0, 5), "bump ")) { // read bump texture
				std::istringstream s(line.substr(5));
				s >> m_Mesh->m_Materials[currentMaterialID].BumpName;
				m_Mesh->m_Materials[currentMaterialID].BumpName = folder + m_Mesh->m_Materials[currentMaterialID].BumpName;
				m_Mesh->m_Materials[currentMaterialID].BumpTexture = AssetManager::RequestMesh(m_Mesh->m_Materials[currentMaterialID].BumpName.c_str());
			}
			else if (line.substr(0, 7) == "map_ns ") { // read specularity
				std::istringstream s(line.substr(7));
				s >> m_Mesh->m_Materials[currentMaterialID].SpecularityName;
				m_Mesh->m_Materials[currentMaterialID].SpecularityName = folder + m_Mesh->m_Materials[currentMaterialID].SpecularityName;
				m_Mesh->m_Materials[currentMaterialID].SpecularityTexture =AssetManager::RequestTexture(m_Mesh->m_Materials[currentMaterialID].SpecularityName.c_str());
			}
			else if (line[0] == '#') { /* ignoring this line */ }
			else { /* ignoring this line */ }
		}
		in.close();
	}

}