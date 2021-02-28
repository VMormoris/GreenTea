#include "Mesh.h"
#include "GraphicsContext.h"

#include <GreenTea/Core/Logger.h>

#include <GreenTea/Assets/AssetManager.h>

namespace GTE::GPU {

	Mesh* Mesh::Create(GeometricMesh* mesh)
	{
		Mesh* gpumesh = new Mesh();
		gpumesh->m_VAO = VertexArray::Create();

		gpumesh->m_ABB = mesh->GetABB();

		gpumesh->m_PositionVBO = VertexBuffer::Create(mesh->GetPositions(), mesh->GetPositionBufferSize());
		gpumesh->m_PositionVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_position"} });
		gpumesh->m_VAO->AddVertexBuffer(gpumesh->m_PositionVBO);

		gpumesh->m_NormalVBO = VertexBuffer::Create(mesh->GetNormals(), mesh->GetNormalBufferSize());
		gpumesh->m_NormalVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_normal"} });
		gpumesh->m_VAO->AddVertexBuffer(gpumesh->m_NormalVBO);

		if (mesh->GetTextCoordBufferSize() != 0)
		{
			gpumesh->m_TextCoordVBO = VertexBuffer::Create(mesh->GetTextCoords(), mesh->GetTextCoordBufferSize());
			gpumesh->m_TextCoordVBO->SetLayout({ {GPU::ShaderDataType::Vec2, "_textCoords"} });
			gpumesh->m_VAO->AddVertexBuffer(gpumesh->m_TextCoordVBO);
		}

		if (mesh->GetTangentsBufferSize() != 0)
		{
			gpumesh->m_TangentsVBO = VertexBuffer::Create(mesh->GetTangents(), mesh->GetTangentsBufferSize());
			gpumesh->m_TangentsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_tangents"} });
			gpumesh->m_VAO->AddVertexBuffer(gpumesh->m_TangentsVBO);
		}

		if (mesh->GetBitangentsBufferSize() != 0)
		{
			gpumesh->m_BitangentsVBO = VertexBuffer::Create(mesh->GetBitangents(), mesh->GetBitangentsBufferSize());
			gpumesh->m_BitangentsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_bitangents"} });
			gpumesh->m_VAO->AddVertexBuffer(gpumesh->m_BitangentsVBO);
		}

		gpumesh->m_Parts = mesh->GetObjects();
		const auto& materials = mesh->GetMaterials();

		for (auto material : materials)
		{
			Material mat;
			mat.Name = material.Name;
			mat.Diffuse = material.Diffuse;
			mat.Ambient = material.Ambient;
			mat.Specular = material.Specular;
			mat.Alpha = material.Alpha;
			mat.Shininess = material.Shininess;
			mat.IlluminationModel = material.IlluminationModel;

			if (!material.DiffuseName.empty())
			{
				mat.DiffuseName = material.DiffuseName;
				mat.DiffuseTexture = CreateRef<Asset>(nullptr, AssetType::LOADING);
			}
			else
				mat.DiffuseTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
			if (!material.NormalName.empty())
			{
				mat.NormalName = material.NormalName;
				mat.NormalTexture = CreateRef<Asset>(nullptr, AssetType::LOADING);
			}
			else
				mat.NormalTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
			if (!material.BumpName.empty())
			{
				mat.BumpName = material.BumpName;
				mat.BumpTexture = CreateRef<Asset>(nullptr, AssetType::LOADING);
			}
			else
				mat.BumpTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
			if (!material.SpecularName.empty())
			{
				mat.SpecularName = material.SpecularName;
				mat.SpecularTexture = CreateRef<Asset>(nullptr, AssetType::LOADING);
			}
			else
				mat.SpecularTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
			if (!material.AmbientName.empty())
			{
				mat.AmbientName = material.AmbientName;
				mat.AmbientTexture = CreateRef<Asset>(nullptr, AssetType::LOADING);
			}
			else
				mat.AmbientTexture = CreateRef<Asset>(nullptr, AssetType::INVALID);
			gpumesh->m_Materials.push_back(mat);
		}

		return gpumesh;
	}

	Mesh* Mesh::Clone(void) const
	{
		return nullptr;
	}


	Mesh::~Mesh(void)
	{
		delete m_VAO;
		if (m_TextCoordVBO != nullptr) delete m_TextCoordVBO;
		delete m_NormalVBO;
		delete m_PositionVBO;
	}

	void Mesh::Bind(void) const { m_VAO->Bind(); }
	void Mesh::Unbind(void) const { m_VAO->Unbind(); }

}