#include "Mesh.h"

#include <Engine/Core/Math.h>

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace gte::GPU {

	Mesh::Mesh(const YAML::Node& data, std::ifstream& ifs)
	{
		using namespace gte::math;
		let& materials = data["Materials"];
		for (let& mat : materials)
			mMaterials.emplace_back(mat.as<std::string>());

		let& parts = data["Parts"];
		for (let& part : parts)
		{
			MeshPart meshpart;
			meshpart.Start = part["Start"].as<uint32>();
			meshpart.End = part["End"].as<uint32>();
			meshpart.MaterialIndex = part["MaterialIndex"].as<int32>();

			mParts.emplace_back(meshpart);
		}

		let& abb = data["ABB"];
		mABB[0] = abb[0].as<glm::vec3>();
		mABB[1] = abb[1].as<glm::vec3>();

		char c;
		ifs.read(&c, 1);

		let vertBuffSize = data["Vertices"].as<size_t>();
		let normBuffSize = data["Normals"].as<size_t>();
		let uvBuffSize = data["UVs"].as<size_t>();
		let tanBuffSize = data["Tangents"].as<size_t>();
		let bitanBuffSize = data["Bitangents"].as<size_t>();

		float* buffer = new float[vertBuffSize];
		mVAO = VertexArray::Create();
		ifs.read((char*)buffer, vertBuffSize);
		mVerticesVBO = VertexBuffer::Create(buffer, vertBuffSize);
		mVerticesVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_position"} });
		mVAO->AddVertexBuffer(mVerticesVBO);

		ifs.read((char*)buffer, normBuffSize);
		mNormalsVBO = VertexBuffer::Create(buffer, normBuffSize);
		mNormalsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_normal"} });
		mVAO->AddVertexBuffer(mNormalsVBO);

		if (uvBuffSize > 0)
		{
			float* uvbuffer = new float[uvBuffSize];
			ifs.read((char*)uvbuffer, uvBuffSize);
			mUVsVBO = VertexBuffer::Create(uvbuffer, uvBuffSize);
			mUVsVBO->SetLayout({ {GPU::ShaderDataType::Vec2, "_textCoords"} });
			mVAO->AddVertexBuffer(mUVsVBO);
		}

		if (tanBuffSize > 0)
		{
			ifs.read((char*)buffer, tanBuffSize);
			mTangentsVBO = VertexBuffer::Create(buffer, tanBuffSize);
			mTangentsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_tangents"} });
			mVAO->AddVertexBuffer(mTangentsVBO);
		}

		if (bitanBuffSize > 0)
		{
			ifs.read((char*)buffer, bitanBuffSize);
			mBitangentsVBO = VertexBuffer::Create(buffer, bitanBuffSize);
			mBitangentsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_bitangents"} });
			mVAO->AddVertexBuffer(mBitangentsVBO);
		}

		delete[] buffer;

#ifndef GT_DIST
		uint32 width, height;
		int32 bpp;
		ifs.read((char*)&width, 4);
		ifs.read((char*)&height, 4);
		ifs.read((char*)&bpp, 4);
		mThumbnail = Image(width, height, bpp);
		ifs.read((char*)mThumbnail.Data(), mThumbnail.Size());
#endif
	}

	Mesh::~Mesh(void)
	{
		delete mVAO;
		if (mUVsVBO != nullptr) delete mUVsVBO;
		delete mNormalsVBO;
		delete mVerticesVBO;
		if (mTangentsVBO) delete mTangentsVBO;
		if (mBitangentsVBO) delete mBitangentsVBO;
	}

	void Mesh::Bind(void) const { mVAO->Bind(); }
	void Mesh::Unbind(void) const { mVAO->Unbind(); }

}