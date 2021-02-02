#pragma once
#include "Texture.h"
#include "VertexArray.h"

#include <GreenTea/Assets/GeometricMesh.h>

namespace GTE{

	namespace GPU {

		class ENGINE_API Mesh {
		public:

			~Mesh(void);

			void Bind(void) const;
			void Unbind(void) const;

			std::vector<MeshPart>& GetParts(void) { return m_Parts; }
			const std::vector<MeshPart>& GetParts(void) const { return m_Parts; }
			
			std::vector<Material>& GetMaterials(void) { return m_Materials; }
			const std::vector<Material>& GetMaterials(void) const { return m_Materials; }

			const GPU::VertexArray* GetVAO(void) const { return m_VAO; }
			static Mesh* Create(GeometricMesh* mesh);
			
			Mesh* Clone(void) const;

		private:

			Mesh(void) = default;

		private:

			std::vector<MeshPart> m_Parts;
			std::vector<Material> m_Materials;
			VertexArray* m_VAO = nullptr;
			VertexBuffer* m_PositionVBO = nullptr;
			VertexBuffer* m_NormalVBO = nullptr;
			VertexBuffer* m_TextCoordVBO = nullptr;
			VertexBuffer* m_TangentsVBO = nullptr;
			VertexBuffer* m_BitangentsVBO = nullptr;
			
		};
	}
}