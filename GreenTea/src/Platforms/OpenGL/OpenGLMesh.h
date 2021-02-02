#pragma once
#include <GreenTea/GPU/Mesh.h>

namespace GTE::GPU::OpenGL {

	class ENGINE_API OpenGLMesh : public Mesh {
	public:
		OpenGLMesh(GeometricMesh* mesh);
		~OpenGLMesh(void);

		void Bind(void) const override;
		void Unbind(void) const override;

		void Draw(void) const override;

	private:


	};

}