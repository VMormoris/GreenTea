//This file is modified version of the code
//	written in the labs of AUEB during the Course of:
//	Computer Graphics 2020 - 2021
#include "OBJLoader.h"

static bool CompareIngoreCase(const std::string& lhs, const std::string& rhs);

gte::Geometry* OBJLoader::Load(const char* filepath)
{
	mVertices.clear();
	mNormals.clear();
	mUVs.clear();
	mElements.clear();
	mFaces.clear();

	mHasTextures = false;
	mMins = glm::vec3{ 999999.0f };
	mMaxes = glm::vec3{ -999999.0f };
	
	char buffer[1024];
	char str[1024];

	let Filepath = std::string(filepath);
	mFolder = std::filesystem::path(filepath).parent_path().string() + "/";

	FILE* file = nullptr;
	file = fopen(filepath, "r");
	ASSERT(file != nullptr, "Error while trying to open: ", filepath);

	if (mMesh)
		delete mMesh;

	mMesh = new gte::Geometry();

	// Create default Part
	int32 pos;
	gte::MeshPart DefaultPart;
	DefaultPart.Start = 0;
	DefaultPart.MaterialIndex = 0;
	mMesh->Parts.push_back(DefaultPart);
	
	int32 currentMaterialIndex = 0;

	// Read file
	while (fgets(buffer, 1024, file) != nullptr)
	{
		if (sscanf(buffer, "%s %n", str, &pos) >= 1)
		{
			if (strcmp(str, "v") == 0) ReadVertices(buffer + pos);
			else if (strcmp(str, "vt") == 0) ReadUVs(buffer + pos);
			else if (strcmp(str, "vn") == 0) ReadNormals(buffer + pos);
			else if (strcmp(str, "f") == 0) ReadFacesLimited(buffer + pos);
			else if (strcmp(str, "usemtl") == 0) ReadMTL(buffer + pos, currentMaterialIndex);
			else if (strcmp(str, "mtllib") == 0) ReadMTLLib(buffer + pos, filepath);
			else if (strcmp(str, "g") == 0 || strcmp(str, "o") == 0) AddGroup(buffer + pos, currentMaterialIndex);
			else if (strcmp(str, "#") == 0) { /* ignoring this line */ }
			else { /* ignoring this line */ }
		}
	}
	fclose(file);

	// Update vertices so the object center is at [0, 0, 0]
	MoveVolume();

	// Generate Vertices from Faces
	GenerateDataFromFaces();

	// Close the last part
	mMesh->Parts.back().End = (uint32)mMesh->Vertices.size();
	mMesh->Parts.erase(std::remove_if(mMesh->Parts.begin(), mMesh->Parts.end(), [](let& part) { return part.Start == part.End; }), mMesh->Parts.end());
	
	if (mMesh->Normals.empty())// Creating normal if they don't exist
		CalculateAverageNormals();
	//if (std::find_if(mMesh->Materials.begin(), mMesh->Materials.end(), [](let& mat) { return mat.Normal.size() > 0; }) != mMesh->Materials.end())
	CalculateTangents();

	return mMesh;
}

void OBJLoader::ReadVertices(const char* buff)
{
	glm::vec3 v;
	char* pEnd;
	v.x = strtof(buff, &pEnd);
	v.y = strtof(pEnd, &pEnd);
	v.z = strtof(pEnd, &pEnd);

	mVertices.emplace_back(v);

	mMins = glm::min(v, mMins);
	mMaxes = glm::max(v, mMaxes);
}

void OBJLoader::MoveVolume(void)
{
	let volCenter = (mMaxes - mMins) / 2.0f + mMins;
	for (auto& pos : mVertices)
		pos -= volCenter;

	mMesh->ABB[0] = mMins - volCenter;
	mMesh->ABB[1] = mMaxes - volCenter;
}

void OBJLoader::ReadUVs(const char* buff)
{
	glm::vec2 uv;
	char* pEnd;
	uv.x = strtof(buff, &pEnd);
	uv.y = strtof(pEnd, &pEnd);

	mUVs.emplace_back(uv);
}

void OBJLoader::ReadNormals(const char* buff)
{
	glm::vec3 n;
	char* pEnd;
	n.x = strtof(buff, &pEnd);
	n.y = strtof(pEnd, &pEnd);
	n.z = strtof(pEnd, &pEnd);

	mNormals.emplace_back(n);
}

void OBJLoader::ReadFaces(const char* buff)
{
	int32 step = 0;
	std::vector<glm::ivec3> VertexIndices;
	VertexIndices.reserve(6);
	glm::ivec3 Indices(0);

	Indices = ReadFaceComponent(buff + step, step);
	while (Indices.x >= 0)// vertex index should always exist
	{
		VertexIndices.emplace_back(Indices);
		Indices = ReadFaceComponent(buff + step, step);
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

	let number_of_faces = (int32)(VertexIndices.size() - 3) + 1;

	for (int32 face = 0; face < number_of_faces; face++)
	{
		//break them using triangle fan :( it is easy for quads
		Face f;
		f.Vertices = glm::ivec3(VertexIndices[face + 0].x, VertexIndices[face + 1].x, VertexIndices[face + 2].x);
		f.Normals = glm::ivec3(VertexIndices[face + 0].y, VertexIndices[face + 1].y, VertexIndices[face + 2].y);
		f.UVs = glm::ivec3(VertexIndices[face + 0].z, VertexIndices[face + 1].z, VertexIndices[face + 2].z);

		mFaces.emplace_back(f);
	}
}

void OBJLoader::ReadFacesLimited(const char* buff)
{
	int32 step = 0;
	glm::ivec3 VerexIndices[4];
	VerexIndices[0] = ReadFaceComponent(buff + step, step);
	VerexIndices[1] = ReadFaceComponent(buff + step, step);
	VerexIndices[2] = ReadFaceComponent(buff + step, step);
	VerexIndices[3] = ReadFaceComponent(buff + step, step);

	// if it is a triangle
	if (VerexIndices[0].x >= 0 && VerexIndices[1].x >= 0 && VerexIndices[2].x >= 0)
	{
		Face face;
		face.Vertices = glm::ivec3(VerexIndices[0].x, VerexIndices[1].x, VerexIndices[2].x);
		face.Normals = glm::ivec3(VerexIndices[0].y, VerexIndices[1].y, VerexIndices[2].y);
		face.UVs = glm::ivec3(VerexIndices[0].z, VerexIndices[1].z, VerexIndices[2].z);
		mFaces.push_back(face);

		// if it was a quad, add another triangle so we can form a quad
		if (VerexIndices[3].x >= 0)
		{
			Face face;
			face.Vertices = glm::ivec3(VerexIndices[2].x, VerexIndices[3].x, VerexIndices[0].x);
			face.Normals = glm::ivec3(VerexIndices[2].y, VerexIndices[3].y, VerexIndices[0].y);
			face.UVs = glm::ivec3(VerexIndices[2].z, VerexIndices[3].z, VerexIndices[0].z);
			mFaces.emplace_back(face);
		}
	}
}

glm::ivec3 OBJLoader::ReadFaceComponent(const char* buff, int32& offset)
{
	int32 index = 0;
	int32 v, vt, vn;

	// v/vt/vn format
	if (sscanf(buff, "%d/%d/%d%n", &v, &vt, &vn, &index) >= 3)
	{
		v += v < 0 ? (int32)mVertices.size() : -1;
		vn += vn < 0 ? (int32)mNormals.size() : -1;
		vt += vt < 0 ? (int32)mUVs.size() : -1;
		mElements.emplace_back(v);
		offset += index;
		return glm::ivec3(v, vn, vt);
	}
	// v//vn format
	if (sscanf(buff, "%d//%d%n", &v, &vn, &index) >= 2)
	{
		v += v < 0 ? (int32)mVertices.size() : -1;
		vn += vn < 0 ? (int32)mNormals.size() : -1;
		mElements.emplace_back(v);
		offset += index;
		return glm::ivec3(v, vn, -1);
	}
	// v/vt format
	if (sscanf(buff, "%d/%d%n", &v, &vt, &index) >= 2)
	{
		v += v < 0 ? (int32)mVertices.size() : -1;
		vt += vt < 0 ? (int32)mUVs.size() : -1;
		mElements.emplace_back(v);
		offset += index;
		return glm::ivec3(v, -1, vt);
	}
	// v format
	if (sscanf(buff, "%d%n", &v, &index) >= 1)
	{
		v += v < 0 ? (int32)mVertices.size() : -1;
		mElements.emplace_back(v);
		offset += index;
		return glm::ivec3(v, -1, -1);
	}
	return glm::ivec3(-1);
}

void OBJLoader::GenerateDataFromFaces(void)
{
	mHasTextures = !mUVs.empty();

	mMesh->Vertices.reserve(mFaces.size() * 3);
	mMesh->Normals.reserve(mFaces.size() * 3);
	if (mHasTextures)
		mMesh->UVs.reserve(mFaces.size() * 3);

	let zero = glm::ivec3{ 0 };
	for (int32 face = 0; face < mFaces.size(); face++)
	{
		let Vertex = mFaces[face].Vertices;
		mMesh->Vertices.emplace_back(mVertices[Vertex.x]);
		mMesh->Vertices.emplace_back(mVertices[Vertex.y]);
		mMesh->Vertices.emplace_back(mVertices[Vertex.z]);

		let Normal = mFaces[face].Normals;
		if (glm::any(glm::greaterThanEqual(Normal, zero)))
		{
			mMesh->Normals.emplace_back(mNormals[Normal.x]);
			mMesh->Normals.emplace_back(mNormals[Normal.y]);
			mMesh->Normals.emplace_back(mNormals[Normal.z]);

			let UV = mFaces[face].UVs;
			if (glm::any(glm::greaterThanEqual(UV, zero)))
			{
				mMesh->UVs.emplace_back(mUVs[UV.x]);
				mMesh->UVs.emplace_back(mUVs[UV.y]);
				mMesh->UVs.emplace_back(mUVs[UV.z]);
			}
			else if(mHasTextures)
			{
				mMesh->UVs.emplace_back(glm::vec2{ 0.0f });
				mMesh->UVs.emplace_back(glm::vec2{ 0.0f });
				mMesh->UVs.emplace_back(glm::vec2{ 0.0f });
			}
		}
	}
}

void OBJLoader::CalculateFlatNormals(void)
{
	for (uint32 i = 0; i < mMesh->Vertices.size(); i++)
	{
		if ((i % 3) == 2)
		{
			let normal = glm::normalize
			(
				glm::cross
				(
					(mMesh->Vertices[i] - mMesh->Vertices[i - 2]),
					(mMesh->Vertices[i - 1] - mMesh->Vertices[i - 2])
				)
			);
			for (int32 j = 0; j < 3; j++)
				mMesh->Normals.push_back(normal);
		}
	}
}

void OBJLoader::CalculateAverageNormals(void)
{
	std::vector<int32> nbseen;
	std::vector<glm::vec3> tnormals;
	tnormals.resize(mVertices.size(), glm::vec3{ 0.0f });
	nbseen.resize(mVertices.size(), 0);

	for (uint32 i = 0; i < mElements.size(); i++)
	{
		uint32 ia = mElements[i];
		uint32 ib = mElements[i + 1];
		uint32 ic = mElements[i + 2];

		let normal = glm::normalize
		(
			glm::cross
			(
				glm::vec3(mVertices[ib] - mVertices[ia]),
				glm::vec3(mVertices[ic] - mVertices[ia])
			)
		);
		
		const int32 v[3] = { (int32)ia, (int32)ib, (int32)ic };
		for (int32 j = 0; j < 3; j++)
		{
			uint32 currv = v[j];
			nbseen[currv]++;
			if (nbseen[currv] == 1)
				tnormals[currv] = normal;
			else
			{
				let fraction = 1.0f / nbseen[currv];
				let mul = (1.0f - fraction);
				tnormals[currv].x = tnormals[currv].x * mul + normal.x * fraction;
				tnormals[currv].y = tnormals[currv].y * mul + normal.y * fraction;
				tnormals[currv].z = tnormals[currv].z * mul + normal.z * fraction;

				tnormals[currv] = glm::normalize(tnormals[currv]);
			}
		}
	}

	//Add normals
	for (uint32 i = 0; i < mElements.size(); i += 3)
	{
		let ia = mElements[i];
		let ib = mElements[i + 1];
		let ic = mElements[i + 2];
		mNormals.push_back(tnormals[ia]);
		mNormals.push_back(tnormals[ib]);
		mNormals.push_back(tnormals[ic]);
	}
}

void OBJLoader::CalculateTangents(void)
{
	mMesh->Tangents.clear();

	for (uint32 i = 0; i < mMesh->Vertices.size(); i += 3)
	{
		let& v0 = mMesh->Vertices[i + 0];
		let& v1 = mMesh->Vertices[i + 1];
		let& v2 = mMesh->Vertices[i + 2];

		let& uv0 = mMesh->UVs[i + 0];
		let& uv1 = mMesh->UVs[i + 1];
		let& uv2 = mMesh->UVs[i + 2];

		// Edges of the triangle : position delta
		let deltapos1 = v1 - v0;
		let deltapos2 = v2 - v0;
		// uv delta
		let deltauv1 = uv1 - uv0;
		let deltauv2 = uv2 - uv0;

		let r = 1.0f / (deltauv1.x * deltauv2.y - deltauv1.y * deltauv2.x);
		let tangent = (deltapos1 * deltauv2.y - deltapos2 * deltauv1.y) * r;
		let b = (deltapos2 * deltauv1.x - deltapos1 * deltauv2.x) * r;

		// Set the same tangent for all three vertices of the triangle
		// Will merge them later on
		mMesh->Tangents.push_back(tangent);
		mMesh->Tangents.push_back(tangent);
		mMesh->Tangents.push_back(tangent);

		mMesh->Bitangents.push_back(b);
		mMesh->Bitangents.push_back(b);
		mMesh->Bitangents.push_back(b);
	}

	for (uint32 i = 0; i < mMesh->Vertices.size(); i++)
	{
		let& n = mMesh->Normals[i];
		let& b = mMesh->Bitangents[i];
		glm::vec3& t = mMesh->Tangents[i];

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));
		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f)
			t *= -1.0f;
	}

}

void OBJLoader::ReadMTL(const char* buff, int32& materialID)
{
	char temp[1024];
	sscanf(buff, "%s", temp);

	// Read Material's name
	std::string str(temp);
	
	// Check if we have already defined a material
	if (mMesh->Parts.back().MaterialIndex > 0)
	{
		// Set where the current part ends
		mMesh->Parts.back().End = 3 * (uint32)mFaces.size();
		// Create new Part
		gte::MeshPart part;
		part.MaterialIndex = mMesh->FindMaterial(str);
		part.Start = 3 * (uint32)mFaces.size();
		mMesh->Parts.emplace_back(part);
	}
	else
		mMesh->Parts.back().MaterialIndex = mMesh->FindMaterial(str);

	materialID = mMesh->Parts.back().MaterialIndex;
}

void OBJLoader::ReadMTLLib(const char* buff, const char* filename)
{
	char temp[1024];
	sscanf(buff, "%s", temp);

	std::string str(temp);
	std::string folder = mFolder;
	str = folder + str;
	ParseMTL(str.c_str());
}

void OBJLoader::AddGroup(const char* buff, int32& materialID)
{
	char name[1024];
	sscanf(buff, "%s", name);

	// End the previous Part
	mMesh->Parts.back().End = 3 * (uint32)mFaces.size();
	
	// Create new Part
	gte::MeshPart part;
	part.Start = 3 * (uint32)mFaces.size();
	part.MaterialIndex = materialID;
	mMesh->Parts.push_back(part);
}

void OBJLoader::ParseMTL(const char* filepath)
{
	std::ifstream ifs(filepath, std::ios::in);
	ASSERT(ifs, "Cannot open Material: ", filepath);
	GTE_TRACE_LOG("Opened Material: ", filepath);

	let folder = std::filesystem::path(filepath).parent_path().string() + "/";
	
	gte::Geometry::Material material;
	material.Name = "Default";
	int32 materialIndex = (int32)mMesh->Materials.size() - 1;
	
	let firtMaterialIndex = materialIndex;
	float r, g, b;
	std::string line;
	while (getline(ifs, line))
	{
		if (line.size() == 0) continue;
		{
			int32 pos = 0;
			while ((line[pos] == ' ' || line[pos] == '\n' || line[pos] == '\r' || line[pos] == '\t') && pos < line.size())
				pos++;
			line = line.substr(pos);
		}

		if (line.substr(0, 7).compare("newmtl ") == 0)// Read Material's name
		{
			std::istringstream s(line.substr(7));
			std::string str;
			s >> str;
			// if it is the default mtl
			str = (str.empty()) ? "default" : str;

			// find if the material with that name exists
			materialIndex = -1;
			for (uint32 i = 0; i < mMesh->Materials.size(); i++)
			{
				if (mMesh->Materials[i].Name.compare(str) == 0)
				{
					materialIndex = i;
					break;
				}
			}
			if (materialIndex == -1) // didn't find the material
			{
				gte::Geometry::Material mat;
				mat.Name = str;
				mMesh->Materials.push_back(mat);
				materialIndex = (int32)mMesh->Materials.size() - 1;
			}
		}
		else if (line.substr(0, 3).compare("Ka ") == 0)
		{
			std::istringstream s(line.substr(3));
			s >> r; s >> g; s >> b;
			mMesh->Materials[materialIndex].AmbientColor = { r, g, b };
		}
		else if (line.substr(0, 3).compare("Kd ") == 0)
		{
			std::istringstream s(line.substr(3));
			s >> r; s >> g; s >> b;
			mMesh->Materials[materialIndex].Diffuse = { r, g, b };
		}
		else if (line.substr(0, 3).compare("Ns ") == 0)
		{
			std::istringstream s(line.substr(3));
			s >> r;
			mMesh->Materials[materialIndex].Roughness = 1.0f - r;
		}
		else if (line.substr(0, 2).compare("d ") == 0)
		{
			std::istringstream s(line.substr(2));
			s >> r;
			mMesh->Materials[materialIndex].Alpha = r;
		}
		else if (line.substr(0, 6).compare("illum ") == 0)
		{
			std::istringstream s(line.substr(6));
			int i;
			s >> i;
			mMesh->Materials[materialIndex].IlluminationModel  = i;
		}
		else if (CompareIngoreCase(line.substr(0, 7), "map_kd "))
		{
			std::istringstream s(line.substr(7));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].Albedo = folder + filename;
			mMesh->Materials[materialIndex].Diffuse = { 1.0f, 1.0f, 1.0f };
		}
		else if (CompareIngoreCase(line.substr(0, 13), "map_emissive "))
		{
			std::istringstream s(line.substr(13));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].Emission = folder + filename;
			mMesh->Materials[materialIndex].EmitColor = { 1.0f, 1.0f, 1.0f };
			mMesh->Materials[materialIndex].IsEmissive = true;
		}
		else if (CompareIngoreCase(line.substr(0, 7), "map_ka "))
		{
			std::istringstream s(line.substr(7));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].AmbientOclussion = folder + filename;
			mMesh->Materials[materialIndex].AmbientColor = { 1.0f, 1.0f, 1.0f };
		}
		else if (CompareIngoreCase(line.substr(0, 7), "map_ks "))
		{
			std::istringstream s(line.substr(7));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].Metallic = folder + filename;
			mMesh->Materials[materialIndex].Metallicness = 1.0f;
			mMesh->Materials[materialIndex].Roughness = 1.0f;
		}
		else if (CompareIngoreCase(line.substr(0, 6), "map_d "))
		{
			std::istringstream s(line.substr(6));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].Opacity = folder + filename;;
			mMesh->Materials[materialIndex].Alpha = 1.0f;
		}
		else if (CompareIngoreCase(line.substr(0, 9), "map_bump "))
		{
			std::istringstream s(line.substr(9));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].Normal = folder + filename;
		}
		else if (CompareIngoreCase(line.substr(0, 5), "bump "))
		{
			std::istringstream s(line.substr(5));
			std::string filename;
			s >> filename;
			mMesh->Materials[materialIndex].Normal = folder + filename;
		}
		else if (line[0] == '#') {} // Ingore this line (Comment)
		else {} // Ignore this line
	}
	ifs.close();
}

OBJLoader::~OBJLoader(void) { }


static bool CompareIngoreCase(const std::string& lhs, const std::string& rhs)
{
	std::string left = "";
	for (let c : lhs)
		left += std::tolower(c);
	std::string right = "";
	for (let c : rhs)
		right += std::tolower(c);

	return left.compare(right) == 0;
}