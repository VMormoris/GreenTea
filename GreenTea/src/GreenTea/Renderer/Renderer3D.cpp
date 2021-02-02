#include "Renderer3D.h"
#include "RenderCommand.h"

#include <GreenTea/GPU/Shader.h>
#include <GreenTea/GPU/FrameBuffer.h>

#include <GreenTea/Assets/AssetManager.h>

namespace GTE {

	static GPU::Shader* s_Shader3D = nullptr;
	static GPU::Shader* s_ShaderShadows = nullptr;
	static GPU::FrameBuffer* s_ShadowmapFBO = nullptr;

	void Renderer3D::Init()
	{
		s_Shader3D = GPU::Shader::Create("../Assets/Shaders/Shader3D.glsl");
		s_Shader3D->Bind();
		//Vertex Shader's uniforms
		s_Shader3D->AddUniform("u_EyeMatrix");
		s_Shader3D->AddUniform("u_ModelMatrix");
		s_Shader3D->AddUniform("u_NormalMatrix");

		//Fragment Shader's uniforms
		s_Shader3D->AddUniform("u_Diffuse");
		s_Shader3D->AddUniform("u_Specular");
		s_Shader3D->AddUniform("u_Ambient");
		s_Shader3D->AddUniform("u_Shininess");
		s_Shader3D->AddUniform("u_HasTexture");

		s_Shader3D->AddUniform("u_HasLight");
		s_Shader3D->AddUniform("u_LightColor");
		s_Shader3D->AddUniform("u_LightPos");
		s_Shader3D->AddUniform("u_LightDir");
		s_Shader3D->AddUniform("u_Umbra");
		s_Shader3D->AddUniform("u_Penumbra");
		s_Shader3D->AddUniform("u_CameraPos");
		s_Shader3D->AddUniform("u_CameraDir");

		s_Shader3D->AddUniform("u_ID");

		s_ShaderShadows = GPU::Shader::Create("../Assets/Shaders/ShaderShadowmap.glsl");
		s_ShaderShadows->AddUniform("u_ProjectionMatrix");

	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, GPU::Mesh* mesh, uint32 ID)
	{
		const glm::mat4 NormalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform))));
		mesh->Bind();
		s_Shader3D->SetUniform("u_ID", ID);
		s_Shader3D->SetUniform("u_ModelMatrix", transform);
		s_Shader3D->SetUniform("u_NormalMatrix", NormalMatrix);

		s_Shader3D->SetUniform("DiffuseTexture", 0);
		s_Shader3D->SetUniform("NormalTexture", 0);

		const auto& parts = mesh->GetParts();
		auto& materials = mesh->GetMaterials();
		for (int32 i = 0; i <parts.size(); i++)
		{
			auto& material = materials[parts[i].MaterialID];

			s_Shader3D->SetUniform("u_Diffuse", material.Diffuse);
			s_Shader3D->SetUniform("u_Ambient", material.Ambient);
			s_Shader3D->SetUniform("u_Specular", material.Specular);
			s_Shader3D->SetUniform("u_Shininess", material.Shininess);

			if (material.DiffuseTexture->Type == AssetType::INVALID)
				s_Shader3D->SetUniform("u_HasTexture", false);
			else if (material.DiffuseTexture->Type == AssetType::LOADING)
			{
				material.DiffuseTexture = AssetManager::RequestTexture(material.DiffuseName.c_str());
				if(material.DiffuseTexture->Type == AssetType::LOADING)
					s_Shader3D->SetUniform("u_HasTexture", false);
			}
			
			if (material.DiffuseTexture->Type == AssetType::TEXTURE)
			{
				s_Shader3D->SetUniform("u_HasTexture", true);
				((GPU::Texture2D*)material.DiffuseTexture->ActualAsset)->Bind(0);
			}

			if (material.NormalTexture->Type == AssetType::INVALID)
				s_Shader3D->SetUniform("u_HasNormal", false);
			else if (material.NormalTexture->Type == AssetType::LOADING)
			{
				material.NormalTexture = AssetManager::RequestTexture(material.NormalName.c_str());
				if (material.NormalTexture->Type == AssetType::LOADING)
					s_Shader3D->SetUniform("u_HasNormal", false);
			}

			if (material.NormalTexture->Type == AssetType::TEXTURE)
			{
				s_Shader3D->SetUniform("u_HasNormal", true);
				((GPU::Texture2D*)material.NormalTexture->ActualAsset)->Bind(1);
			}

			if (material.BumpTexture->Type == AssetType::INVALID)
				s_Shader3D->SetUniform("u_HasNormal", false);
			else if (material.BumpTexture->Type == AssetType::LOADING)
			{
				material.BumpTexture = AssetManager::RequestTexture(material.BumpName.c_str());
				if (material.BumpTexture->Type == AssetType::LOADING)
					s_Shader3D->SetUniform("u_HasNormal", false);
			}

			if (material.BumpTexture->Type == AssetType::TEXTURE)
			{
				s_Shader3D->SetUniform("u_HasNormal", true);
				s_Shader3D->SetUniform("u_IsBump", true);
				((GPU::Texture2D*)material.BumpTexture->ActualAsset)->Bind(1);
			}

			RenderCommand::DrawArray(mesh->GetVAO(), parts[i].Start, (parts[i].End - parts[i].Start));
		}
	}


	void Renderer3D::BeginScene(const SceneRenderData& data)
	{
		s_Shader3D->Bind();
		s_Shader3D->SetUniform("u_EyeMatrix", data.EyeMatrix);

		if (data.lc)
		{
			s_Shader3D->SetUniform("u_HasLight", true);
			s_Shader3D->SetUniform("u_LightColor", data.lc->Color * data.lc->Intensity);
			s_Shader3D->SetUniform("u_LightPos", data.LightPos);
			s_Shader3D->SetUniform("u_LightDir", data.lc->Direction);
			s_Shader3D->SetUniform("u_Umbra", data.lc->Umbra);
			s_Shader3D->SetUniform("u_Penumbra", data.lc->Penumbra);
			s_Shader3D->SetUniform("u_CameraPos", data.CameraPos);
			s_Shader3D->SetUniform("u_CameraDir", data.CameraDir);
		}
		else
			s_Shader3D->SetUniform("u_HasLight", false);
	}

	void Renderer3D::EndScene(void) { }

	void Renderer3D::Shutdown(void)
	{ 
		delete s_Shader3D;
		delete s_ShaderShadows;
		delete s_ShadowmapFBO;
	}

}