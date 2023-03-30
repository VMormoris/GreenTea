#pragma once

#include "VertexArray.h"
#include <Engine/Assets/Material.h>
#include <Engine/Assets/Geometry.h>

#include <array>

// Forward Declaration(s)
namespace YAML { class Node; }

namespace gte::GPU {

	class ENGINE_API Mesh {
	public:
		~Mesh(void);

		void Bind(void) const;
		void Unbind(void) const;

		std::vector<MeshPart>& GetParts(void) { return mParts; }
		const std::vector<MeshPart>& GetParts(void) const { return mParts; }

		std::vector<uuid>& GetMaterials(void) { return mMaterials; }
		const std::vector<uuid>& GetMaterials(void) const { return mMaterials; }

		const GPU::VertexArray* GetVAO(void) const { return mVAO; }

		std::array<glm::vec3, 2>& GetABB(void) { return mABB; }
		const std::array<glm::vec3, 2>& GetABB(void) const { return mABB; }

		Mesh(const YAML::Node& data, std::ifstream& ifs);

	private:

		std::vector<MeshPart> mParts;
		std::vector<uuid> mMaterials;

		VertexArray* mVAO = nullptr;
		VertexBuffer* mVerticesVBO = nullptr;
		VertexBuffer* mNormalsVBO = nullptr;
		VertexBuffer* mUVsVBO = nullptr;
		VertexBuffer* mTangentsVBO = nullptr;
		VertexBuffer* mBitangentsVBO = nullptr;

		std::array<glm::vec3, 2> mABB = { glm::vec3(999999.0f), glm::vec3(-999999.0f) };
	};

}

