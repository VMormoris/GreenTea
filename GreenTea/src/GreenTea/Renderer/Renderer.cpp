#include "Renderer.h"
#include "RenderCommand.h"

#include <GreenTea/GPU/Shader.h>

#include <gtc/matrix_transform.hpp>




namespace GTE {

	//Utility function for calculating Projection * View (aka Eye Matrix) matrix of any light
	glm::mat4 CalculateLightEyeMatrix(const GTE::LightSource& light);

	struct MeshNode {
		const glm::mat4* Transform = nullptr;
		GPU::Mesh* Geometry = nullptr;
		uint32 ID = entt::null;

		MeshNode(const glm::mat4& transform, GPU::Mesh* geometry, uint32 id)
			: Transform(&transform), Geometry(geometry), ID(id) {}

	};

	struct Renderer3DData{
		
		GPU::Shader* Shader3D = nullptr;
		GPU::Shader* ShaderShadows = nullptr;
		GPU::Shader* ShaderSkybox = nullptr;

		GPU::VertexArray* SkyboxVAO = nullptr;
		GPU::VertexBuffer* SkyboxVBO = nullptr;

		GPU::FrameBuffer* ShadowmapFBO = nullptr;
		SceneData SceneData;

		std::array<glm::vec3, 2> ABB = { glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f} };
		
		std::vector<MeshNode> Meshes;
		std::vector<LightSource> Lights;
	};

	static Renderer3DData s_RendererData;

	void Renderer::Init(void)
	{
		s_RendererData.Shader3D = GPU::Shader::Create("../Assets/Shaders/PBR.glsl");
		s_RendererData.Shader3D->Bind();
		//Vertex Shader's uniforms
		s_RendererData.Shader3D->AddUniform("u_EyeMatrix");
		s_RendererData.Shader3D->AddUniform("u_ModelMatrix");
		s_RendererData.Shader3D->AddUniform("u_NormalMatrix");

		//Fragment Shader's uniforms
		s_RendererData.Shader3D->AddUniform("u_Diffuse");
		s_RendererData.Shader3D->AddUniform("u_Specular");
		s_RendererData.Shader3D->AddUniform("u_Ambient");
		s_RendererData.Shader3D->AddUniform("u_Shininess");

		s_RendererData.Shader3D->AddUniform("u_HasTexture");
		s_RendererData.Shader3D->AddUniform("u_HasLight");
		s_RendererData.Shader3D->AddUniform("u_HasNormal");
		s_RendererData.Shader3D->AddUniform("u_HasMask");
		s_RendererData.Shader3D->AddUniform("u_HasEmissive");
		//s_RendererData.Shader3D->AddUniform("u_IsBump");

		s_RendererData.Shader3D->AddUniform("u_LightType");
		s_RendererData.Shader3D->AddUniform("u_LightColor");
		s_RendererData.Shader3D->AddUniform("u_LightPos");
		s_RendererData.Shader3D->AddUniform("u_LightDir");
		s_RendererData.Shader3D->AddUniform("u_Umbra");
		s_RendererData.Shader3D->AddUniform("u_Penumbra");
		s_RendererData.Shader3D->AddUniform("u_LightProjectionMatrix");
		s_RendererData.Shader3D->AddUniform("u_ConstantBias");
		
		s_RendererData.Shader3D->AddUniform("u_CameraPos");
		s_RendererData.Shader3D->AddUniform("u_CameraDir");

		s_RendererData.Shader3D->AddUniform("u_ID");
		s_RendererData.Shader3D->AddUniform("u_ViewportSize");

		s_RendererData.Shader3D->AddUniform("DiffuseTexture");
		s_RendererData.Shader3D->AddUniform("NormalTexture");
		s_RendererData.Shader3D->AddUniform("MaskTexture");
		s_RendererData.Shader3D->AddUniform("EmissiveTexture");
		s_RendererData.Shader3D->AddUniform("ShadowmapTexture");

		s_RendererData.ShaderShadows = GPU::Shader::Create("../Assets/Shaders/ShaderShadowmap.glsl");
		s_RendererData.ShaderShadows->AddUniform("u_EyeModelMatrix");

		s_RendererData.ShaderSkybox = GPU::Shader::Create("../Assets/Shaders/Skybox.glsl");

		s_RendererData.ShaderSkybox->AddUniform("u_EyeModelMatrix");
		s_RendererData.ShaderSkybox->AddUniform("Skybox");

		GPU::FrameBufferSpecification spec;
		spec.Attachments = { GPU::TextureFormat::Shadowmap };
		spec.Width = 1024;
		spec.Height = 1024;
		s_RendererData.ShadowmapFBO = GPU::FrameBuffer::Create(spec);

		constexpr glm::vec3 skyboxVertices[] =
		{
			//Front Face
			{-0.5f, -0.5f, -0.5f},//Left Bottom Corner
			{ 0.5f, -0.5f, -0.5f},//Right Bottom Corner
			{ 0.5f,  0.5f, -0.5f},//Right Top Corner
			{ 0.5f,  0.5f, -0.5f},//Right Top Corner
			{-0.5f,  0.5f, -0.5f},//Left Top Corner
			{-0.5f, -0.5f, -0.5f},//Left Bottom Corner

			//Left Face
			{-0.5f, -0.5f, -0.5f},//Bottom Front Corner
			{-0.5f, -0.5f,  0.5f},//Bottom Back Corner
			{-0.5f,  0.5f,  0.5f},//Top Back Corner
			{-0.5f,  0.5f,  0.5f},//Top Back Corner
			{-0.5f,  0.5f, -0.5f},//Top Front Corner
			{-0.5f, -0.5f, -0.5f},//Bottom Front Corner

			//Bottom Face
			{-0.5f, -0.5f, -0.5f},//Left Front Corner
			{ 0.5f, -0.5f, -0.5f},//Right Front Corner
			{ 0.5f, -0.5f, 0.5f},//Right Back Corner
			{ 0.5f, -0.5f, 0.5f},//Right Back Corner
			{-0.5f, -0.5f, 0.5f},//Left Back Corner
			{-0.5f, -0.5f, -0.5f},//Left Front Corner

			//Back Face
			{-0.5f, -0.5f, 0.5f},//Left Bottom Corner
			{ 0.5f, -0.5f, 0.5f},//Right Bottom Corner
			{ 0.5f,  0.5f, 0.5f},//Right Top Corner
			{ 0.5f,  0.5f, 0.5f},//Right Top Corner
			{-0.5f,  0.5f, 0.5f},//Left Top Corner
			{-0.5f, -0.5f, 0.5f},//Left Bottom Corner

			//Right Face
			{ 0.5f, -0.5f, -0.5f},//Bottom Front Corner
			{ 0.5f, -0.5f,  0.5f},//Bottom Back Corner
			{ 0.5f,  0.5f,  0.5f},//Top Back Corner
			{ 0.5f,  0.5f,  0.5f},//Top Back Corner
			{ 0.5f,  0.5f, -0.5f},//Top Front Corner
			{ 0.5f, -0.5f, -0.5f},//Bottom Front Corner

			//Top Face
			{-0.5f,  0.5f, -0.5f},//Left Front Corner
			{ 0.5f,  0.5f, -0.5f},//Right Front Corner
			{ 0.5f,  0.5f, 0.5f},//Right Back Corner
			{ 0.5f,  0.5f, 0.5f},//Right Back Corner
			{-0.5f,  0.5f, 0.5f},//Left Back Corner
			{-0.5f,  0.5f, -0.5f},//Left Front Corner
		};

		s_RendererData.SkyboxVAO = GPU::VertexArray::Create();

		s_RendererData.SkyboxVBO = GPU::VertexBuffer::Create(skyboxVertices, sizeof(skyboxVertices));
		s_RendererData.SkyboxVBO->SetLayout({ { GPU::ShaderDataType::Vec3, "_position" } });

		s_RendererData.SkyboxVAO->AddVertexBuffer(s_RendererData.SkyboxVBO);
	}

	void Renderer::EndScene(const GPU::CubicTexture* skybox)
	{
		for (const auto& light : s_RendererData.Lights)
		{
			RenderShadowmaps(light);
			RenderGeometry(light);
		}

		if (skybox)
			RenderSkybox(skybox);
	}

	void Renderer::RenderGeometry(const LightSource& light)
	{
		s_RendererData.Shader3D->Bind();

		//Camera's Uniforms
		s_RendererData.Shader3D->SetUniform("u_EyeMatrix", s_RendererData.SceneData.EyeMatrix);
		s_RendererData.Shader3D->SetUniform("u_CameraPos", s_RendererData.SceneData.CameraPos);
		s_RendererData.Shader3D->SetUniform("u_CameraDir", s_RendererData.SceneData.CameraDir);

		//Light's uniforms
		s_RendererData.Shader3D->SetUniform("u_LightType", static_cast<int32>(light.lc->Type));
		s_RendererData.Shader3D->SetUniform("u_LightColor", light.lc->Color * light.lc->Intensity);
		s_RendererData.Shader3D->SetUniform("u_LightPos", light.Position);
		s_RendererData.Shader3D->SetUniform("u_LightDir", light.lc->Direction);
		s_RendererData.Shader3D->SetUniform("u_Umbra", light.lc->Umbra);
		s_RendererData.Shader3D->SetUniform("u_Penumbra", light.lc->Penumbra);
		s_RendererData.Shader3D->SetUniform("u_ConstantBias", light.lc->ShadowMapBias);
		const glm::mat4 LightMatrix = CalculateLightEyeMatrix(light);
		s_RendererData.Shader3D->SetUniform("u_LightProjectionMatrix", LightMatrix);
		
		//Sampler uniforms
		s_RendererData.Shader3D->SetUniform("DiffuseTexture", 0);
		s_RendererData.Shader3D->SetUniform("NormalTexture", 1);
		s_RendererData.Shader3D->SetUniform("MaskTexture", 2);
		s_RendererData.Shader3D->SetUniform("EmissiveTexture", 3);
		s_RendererData.Shader3D->SetUniform("ShadowmapTexture", 4);
		s_RendererData.ShadowmapFBO->BindAttachment(0, 4);

		s_RendererData.SceneData.Target->Bind();
		const auto& fboSpec = s_RendererData.SceneData.Target->GetSpecification();
		RenderCommand::SetViewport(0, 0, fboSpec.Width, fboSpec.Height);
		RenderCommand::SetClearColor({ 1.0f, 0.0f, 1.0f, 1.0f });
		RenderCommand::Clear();
		s_RendererData.Shader3D->SetUniform("u_ViewportSize", glm::vec2(fboSpec.Width, fboSpec.Height));

		for (const auto& mesh : s_RendererData.Meshes)
		{
			const glm::mat4 NormalMatrix = glm::mat4(glm::transpose(glm::inverse(*mesh.Transform)));
			mesh.Geometry->Bind();
			s_RendererData.Shader3D->SetUniform("u_ID", mesh.ID);
			s_RendererData.Shader3D->SetUniform("u_ModelMatrix", *mesh.Transform);
			s_RendererData.Shader3D->SetUniform("u_NormalMatrix", NormalMatrix);

			const auto& parts = mesh.Geometry->GetParts();
			auto& materials = mesh.Geometry->GetMaterials();
			for (int32 i = 0; i < parts.size(); i++)
			{
				auto& material = materials[parts[i].MaterialID];

				s_RendererData.Shader3D->SetUniform("u_Diffuse", material.Diffuse);
				s_RendererData.Shader3D->SetUniform("u_Ambient", material.Ambient);
				s_RendererData.Shader3D->SetUniform("u_Specular", material.Specular);
				s_RendererData.Shader3D->SetUniform("u_Shininess", material.Shininess);

				{//Albedo
					if (material.DiffuseTexture->Type == AssetType::INVALID)
						s_RendererData.Shader3D->SetUniform("u_HasTexture", false);
					else if (material.DiffuseTexture->Type == AssetType::LOADING)
					{
						material.DiffuseTexture = AssetManager::RequestTexture(material.DiffuseName.c_str());
						if (material.DiffuseTexture->Type == AssetType::LOADING)
							s_RendererData.Shader3D->SetUniform("u_HasTexture", false);
					}

					if (material.DiffuseTexture->Type == AssetType::TEXTURE)
					{
						s_RendererData.Shader3D->SetUniform("u_HasTexture", true);
						((GPU::Texture2D*)material.DiffuseTexture->ActualAsset)->Bind(0);
					}
				}

				{//Normal
					if (material.NormalTexture->Type == AssetType::INVALID)
						s_RendererData.Shader3D->SetUniform("u_HasNormal", false);
					else if (material.NormalTexture->Type == AssetType::LOADING)
					{
						material.NormalTexture = AssetManager::RequestTexture(material.NormalName.c_str());
						if (material.NormalTexture->Type == AssetType::LOADING)
							s_RendererData.Shader3D->SetUniform("u_HasNormal", false);
					}

					if (material.NormalTexture->Type == AssetType::TEXTURE)
					{
						s_RendererData.Shader3D->SetUniform("u_HasNormal", true);
						((GPU::Texture2D*)material.NormalTexture->ActualAsset)->Bind(1);
					}
				}

				{//Metallicity Ambient Reflectance Glossiness
					if (material.SpecularTexture->Type == AssetType::INVALID)
						s_RendererData.Shader3D->SetUniform("u_HasMask", false);
					else if (material.SpecularTexture->Type == AssetType::LOADING)
					{
						material.SpecularTexture = AssetManager::RequestTexture(material.SpecularName.c_str());
						if(material.SpecularTexture->Type == AssetType::LOADING)
							s_RendererData.Shader3D->SetUniform("u_HasMask", false);
					}

					if (material.SpecularTexture->Type == AssetType::TEXTURE)
					{
						s_RendererData.Shader3D->SetUniform("u_HasMask", true);
						((GPU::Texture2D*)material.SpecularTexture->ActualAsset)->Bind(2);
					}
				}

				{//Emissivity
					if (material.AmbientTexture->Type == AssetType::INVALID)
						s_RendererData.Shader3D->SetUniform("u_HasEmissive", false);
					else if (material.AmbientTexture->Type == AssetType::LOADING)
					{
						material.AmbientTexture = AssetManager::RequestTexture(material.AmbientName.c_str());
						if (material.AmbientTexture->Type == AssetType::LOADING)
							s_RendererData.Shader3D->SetUniform("u_HasEmissive", false);
					}

					if (material.AmbientTexture->Type == AssetType::TEXTURE)
					{
						s_RendererData.Shader3D->SetUniform("u_HasEmissive", true);
						((GPU::Texture2D*)material.AmbientTexture->ActualAsset)->Bind(3);
					}
				}

				RenderCommand::DrawArray(mesh.Geometry->GetVAO(), parts[i].Start, (parts[i].End - parts[i].Start));
			}
		}
	}

	void Renderer::RenderShadowmaps(const LightSource& light)
	{
		//s_RendererData.ShadowmapFBO->Resize(light.lc->ShadowMapResolution, light.lc->ShadowMapResolution);
		s_RendererData.ShadowmapFBO->Bind();
		auto spec = s_RendererData.ShadowmapFBO->GetSpecification();
		RenderCommand::SetViewport(0, 0, spec.Width, spec.Height);
		RenderCommand::Clear();

		s_RendererData.ShaderShadows->Bind();

		const glm::mat4 EyeMatrix = CalculateLightEyeMatrix(light);
		for (const auto& mesh : s_RendererData.Meshes)
		{
			mesh.Geometry->Bind();
			s_RendererData.ShaderShadows->SetUniform("u_EyeModelMatrix", EyeMatrix * (*mesh.Transform));
			for (const auto& part : mesh.Geometry->GetParts())
				RenderCommand::DrawArray(mesh.Geometry->GetVAO(), part.Start, part.End - part.Start);
		}

		s_RendererData.ShaderShadows->Unbind();
		s_RendererData.ShadowmapFBO->Unbind();
	}

	void Renderer::RenderSkybox(const GPU::CubicTexture* skybox)
	{
		s_RendererData.ShaderSkybox->Bind();
		s_RendererData.SceneData.Target->Bind();
		const glm::mat4 view = glm::mat4(glm::mat3(s_RendererData.SceneData.ViewMatrix));
		const glm::mat4 EyeModelMatrix = s_RendererData.SceneData.ProjectionMatrix * view * glm::mat4(1.0f);
		s_RendererData.ShaderSkybox->SetUniform("u_EyeModelMatrix", EyeModelMatrix);
		s_RendererData.ShaderSkybox->SetUniform("Skybox", 0);
		skybox->Bind(0);
		s_RendererData.SkyboxVAO->Bind();
		RenderCommand::DrawArray(s_RendererData.SkyboxVAO, 0, 36);
	}

	void Renderer::BeginScene(const SceneData& data)
	{ 
		s_RendererData.Meshes.clear();
		s_RendererData.Lights.clear();
		s_RendererData.SceneData = data;
		s_RendererData.ABB[0] = glm::vec3(FLT_MAX);
		s_RendererData.ABB[1] = glm::vec3(FLT_MIN);
	}

	void Renderer::Shutdown(void)
	{
		delete s_RendererData.Shader3D;
		delete s_RendererData.ShaderShadows;
		delete s_RendererData.ShaderSkybox;

		delete s_RendererData.SkyboxVAO;
		delete s_RendererData.SkyboxVBO;

		delete s_RendererData.ShadowmapFBO;
	}

	void Renderer::SubmitMesh(const glm::mat4& transform, GPU::Mesh* mesh, uint32 ID)
	{
		MeshNode node = { transform, mesh, ID };
		std::array<glm::vec3, 2> abb = mesh->GetABB();
		abb[0] = transform * glm::vec4(abb[0].x, abb[0].y, abb[0].z, 1.0f/*Dont Care*/);
		abb[1] = transform * glm::vec4(abb[1].x, abb[1].y, abb[1].z, 1.0f/*Dont Care*/);

		//Now the bounding box is not alligned to the world axis (aka is OBB)
		//	so we need to make ABB again
		float temp = abb[0].x;
		abb[0].x = std::min(temp, abb[1].x);
		abb[1].x = std::max(temp, abb[1].x);

		temp = abb[0].y;
		abb[0].y = std::min(temp, abb[1].y);
		abb[1].y = std::max(temp, abb[1].y);

		temp = abb[0].z;
		abb[0].z = std::min(temp, abb[1].z);
		abb[1].z = std::max(temp, abb[1].z);

		s_RendererData.ABB[0].x = std::min(s_RendererData.ABB[0].x, abb[0].x);
		s_RendererData.ABB[1].x = std::max(s_RendererData.ABB[1].x, abb[1].x);

		s_RendererData.ABB[0].y = std::min(s_RendererData.ABB[0].y, abb[0].y);
		s_RendererData.ABB[1].y = std::max(s_RendererData.ABB[1].y, abb[1].y);

		s_RendererData.ABB[0].z = std::min(s_RendererData.ABB[0].z, abb[0].z);
		s_RendererData.ABB[1].z = std::max(s_RendererData.ABB[1].z, abb[1].z);

		s_RendererData.Meshes.push_back(node);	
	}
	void Renderer::SubmitLight(const glm::vec3& position, const LightComponent& lc) { s_RendererData.Lights.push_back({ position, lc }); }
	void Renderer::ResizeShadowmapRes(const glm::vec2& resolution) { s_RendererData.ShadowmapFBO->Resize(static_cast<uint32>(resolution.x), static_cast<uint32>(resolution.y)); }


	glm::mat4 CalculateLightEyeMatrix(const GTE::LightSource& light)
	{
		glm::mat4 ViewMatrix, ProjectionMatrix;
		switch (light.lc->Type){
		case GTE::LightType::Directional:
		{
			const float dist = glm::distance(s_RendererData.ABB[0], s_RendererData.ABB[1]);
			const float halfDist = dist / 2.0f;
			const glm::vec3 target = (s_RendererData.ABB[1] + s_RendererData.ABB[0]) / 2.0f;//Volumetric Center of ABB
			const glm::vec3 pos = target - halfDist * light.lc->Direction;
			if (light.lc->Direction == glm::vec3{ 0.0f, 1.0f, 0.0f })
				ViewMatrix = glm::lookAt(pos, target, { 0.0f, 0.0f, -1.0f });
			else if (light.lc->Direction == glm::vec3{ 0.0f, -1.0f, 0.0f })
				ViewMatrix = glm::lookAt(pos, target, { 0.0f, 0.0f, 1.0f });
			else
				ViewMatrix = glm::lookAt(pos, target, { 0.0f, 1.0f, 0.0f });
			ProjectionMatrix = glm::ortho(-halfDist, halfDist, -halfDist, halfDist, 0.0f, dist);
			break;
		}
		case LightType::SpotLight:
		{
			ViewMatrix = glm::lookAt(light.Position, light.lc->Target, { 0.0f, 1.0f, 0.0f });
			const float h = light.lc->Near * glm::tan(glm::radians(light.lc->Penumbra * 0.5f));
			ProjectionMatrix = glm::frustum(-h, h, -h, h, light.lc->Near, light.lc->Far);
			break;
		}
		default:
			ENGINE_ASSERT(false, "Not implemented yet!");
		}
		return ProjectionMatrix * ViewMatrix;
	}
}


