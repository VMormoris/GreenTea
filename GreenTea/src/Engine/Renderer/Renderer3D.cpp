#include "Renderer3D.h"
#include "RenderCommand.h"

#include <Engine/Core/Context.h>
#include <Engine/GPU/Shader.h>

#include <gtc/matrix_transform.hpp>
#include <glad/glad.h>

namespace gte {

	struct MeshNode {
		const glm::mat4* Transform = nullptr;
		const GPU::Mesh* Geometry = nullptr;
		uint32 ID = entt::null;

		MeshNode(const glm::mat4& transform, GPU::Mesh* geometry, uint32 id)
			: Transform(&transform), Geometry(geometry), ID(id) {}

	};

	struct Renderer3DData {
		GPU::VertexArray* CubeVAO = nullptr;
		GPU::VertexBuffer* CubeVBO = nullptr;
		
		GPU::Shader* ThumbnailShader = nullptr;
		GPU::Shader* EquirectangularToCubemap = nullptr;
		GPU::Shader* SkyboxShader = nullptr;

		SceneData Data;
		std::array<glm::vec3, 2> ABB = { glm::vec3{FLT_MAX}, glm::vec3{FLT_MIN} };

		std::vector<MeshNode> Meshes;
	};

	static Renderer3DData sRendererData;

	void Renderer3D::Init(void)
	{
		sRendererData.ThumbnailShader = GPU::Shader::Create("../Assets/Shaders/Thumbnail.glsl");
		sRendererData.ThumbnailShader->Bind();
		
		// Vertex Shader's uniforms
		sRendererData.ThumbnailShader->AddUniform("u_EyeMatrix");
		sRendererData.ThumbnailShader->AddUniform("u_ModelMatrix");
		sRendererData.ThumbnailShader->AddUniform("u_NormalMatrix");

		// Fragment Shader's uniforms
		sRendererData.ThumbnailShader->AddUniform("u_Diffuse");
		sRendererData.ThumbnailShader->AddUniform("u_EmitColor");
		sRendererData.ThumbnailShader->AddUniform("u_AmbientColor");
		sRendererData.ThumbnailShader->AddUniform("u_Metallicness");
		sRendererData.ThumbnailShader->AddUniform("u_Roughness");

		sRendererData.ThumbnailShader->AddUniform("u_HasAlbedo");
		sRendererData.ThumbnailShader->AddUniform("u_HasNormal");
		sRendererData.ThumbnailShader->AddUniform("u_HasMetallic");
		sRendererData.ThumbnailShader->AddUniform("u_HasRough");
		sRendererData.ThumbnailShader->AddUniform("u_HasEmissive");
		sRendererData.ThumbnailShader->AddUniform("u_HasOcclusion");
		sRendererData.ThumbnailShader->AddUniform("u_HasOpacity");

		sRendererData.ThumbnailShader->AddUniform("u_CameraPos");
		sRendererData.ThumbnailShader->AddUniform("u_CameraDir");

		sRendererData.ThumbnailShader->AddUniform("u_ViewportSize");

		sRendererData.ThumbnailShader->AddUniform("AlbedoTexture");
		sRendererData.ThumbnailShader->AddUniform("NormalTexture");
		sRendererData.ThumbnailShader->AddUniform("MetallicTexture");
		sRendererData.ThumbnailShader->AddUniform("RoughTexture");
		sRendererData.ThumbnailShader->AddUniform("OclussionTexture");
		sRendererData.ThumbnailShader->AddUniform("OpacityTexture");
		sRendererData.ThumbnailShader->AddUniform("EmissiveTexture");

		sRendererData.EquirectangularToCubemap = GPU::Shader::Create("../Assets/Shaders/EquirectangularToCubemap.glsl");
		sRendererData.EquirectangularToCubemap->AddUniform("u_EyeMatrix");
		sRendererData.EquirectangularToCubemap->AddUniform("EquirectangularMap");

		sRendererData.SkyboxShader = GPU::Shader::Create("../Assets/Shaders/Skybox.glsl");
		sRendererData.SkyboxShader->AddUniform("u_EyeMatrix");
		sRendererData.SkyboxShader->AddUniform("EnvironmentMap");

		constexpr glm::vec3 cubeVertices[] =
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

		sRendererData.CubeVAO = GPU::VertexArray::Create();
		sRendererData.CubeVBO = GPU::VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
		sRendererData.CubeVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_position"} });
		sRendererData.CubeVAO->AddVertexBuffer(sRendererData.CubeVBO);


		//TODO(Vasilis): Maybe move to CreateContext
		GPU::FrameBufferSpecification spec;
		spec.Width = 512; spec.Height = 512;
		spec.Attachments = { GPU::TextureFormat::Cube, GPU::TextureFormat::Depth };
		internal::GetContext()->EnvironmentFBO = GPU::FrameBuffer::Create(spec);
	}

	void Renderer3D::RenderGeometry(void)
	{
		sRendererData.ThumbnailShader->Bind();
		
		//Camera's Uniforms
		sRendererData.ThumbnailShader->SetUniform("u_EyeMatrix", sRendererData.Data.EyeMatrix);
		sRendererData.ThumbnailShader->SetUniform("u_CameraPos", sRendererData.Data.CameraPos);
		sRendererData.ThumbnailShader->SetUniform("u_CameraDir", sRendererData.Data.CameraDir);

		//Sampler uniforms
		sRendererData.ThumbnailShader->SetUniform("AlbedoTexture", 0);
		sRendererData.ThumbnailShader->SetUniform("NormalTexture", 1);
		sRendererData.ThumbnailShader->SetUniform("MetallicTexture", 2);
		sRendererData.ThumbnailShader->SetUniform("RoughTexture", 3);
		sRendererData.ThumbnailShader->SetUniform("OclussionTexture", 4);
		sRendererData.ThumbnailShader->SetUniform("OpacityTexture", 5);
		sRendererData.ThumbnailShader->SetUniform("EmissiveTexture", 6);

		sRendererData.Data.Target->Bind();
		let& fboSpec = sRendererData.Data.Target->GetSpecification();
		RenderCommand::SetViewport(0, 0, fboSpec.Width, fboSpec.Height);
		sRendererData.ThumbnailShader->SetUniform("u_ViewportSize", glm::vec2(fboSpec.Width, fboSpec.Height));
		for (let& mesh : sRendererData.Meshes)
		{
			let NormalMatrix = glm::mat4(glm::transpose(glm::inverse(*mesh.Transform)));
			sRendererData.ThumbnailShader->SetUniform("u_ModelMatrix", *mesh.Transform);
			sRendererData.ThumbnailShader->SetUniform("u_NormalMatrix", NormalMatrix);

			let& parts = mesh.Geometry->GetParts();
			let& materials = mesh.Geometry->GetMaterials();
			for (let& part : parts)
			{
				let& id = materials[part.MaterialIndex];
				Ref<Asset> material = internal::GetContext()->AssetManager.RequestAsset(id);
				if (material->Type != AssetType::MATERIAL)//Not loaded or assign material
					continue;

				let* mat = (Material*)material->Data;
				sRendererData.ThumbnailShader->SetUniform("u_Diffuse", mat->Diffuse);
				sRendererData.ThumbnailShader->SetUniform("u_EmitColor", mat->EmitColor);
				sRendererData.ThumbnailShader->SetUniform("u_AmbientColor", mat->AmbientColor);
				sRendererData.ThumbnailShader->SetUniform("u_Metallicness", mat->Metallicness);
				sRendererData.ThumbnailShader->SetUniform("u_Roughness", mat->Roughness);

				Ref<Asset> albedoTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Albedo->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasAlbedo", albedoTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* albedo = (GPU::Texture*)albedoTexture->Data)
					albedo->Bind(0);

				Ref<Asset> normalTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Normal->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasNormal", normalTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* normal = (GPU::Texture*)normalTexture->Data)
					normal->Bind(1);

				Ref<Asset> metallicTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Metallic->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasMetallic", metallicTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* metallic = (GPU::Texture*)metallicTexture->Data)
					metallic->Bind(2);

				Ref<Asset> roughTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Metallic->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasRough", roughTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* rough = (GPU::Texture*)roughTexture->Data)
					rough->Bind(3);

				Ref<Asset> aoTexture = internal::GetContext()->AssetManager.RequestAsset(mat->AmbientOclussion->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasOcclusion", aoTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* ao = (GPU::Texture*)aoTexture->Data)
					ao->Bind(4);

				Ref<Asset> opacityTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Opacity->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasOpacity", opacityTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* opacity = (GPU::Texture*)opacityTexture->Data)
					opacity->Bind(5);

				Ref<Asset> emmisiveTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Emission->ID);
				sRendererData.ThumbnailShader->SetUniform("u_HasEmissive", emmisiveTexture->Type == AssetType::TEXTURE);
				if (GPU::Texture* emmisive = (GPU::Texture*)emmisiveTexture->Data)
					emmisive->Bind(6);

				let count = part.End - part.Start;
				RenderCommand::DrawArrays(mesh.Geometry->GetVAO(), part.Start, count);
			}
		}
		sRendererData.Data.Target->Unbind();
		sRendererData.ThumbnailShader->Unbind();
	}

	void Renderer3D::SubmitMesh(const glm::mat4& transform, GPU::Mesh* mesh, uint32 ID)
	{
		MeshNode node = { transform, mesh, ID };
		auto abb = mesh->GetABB();
		abb[0] = transform * glm::vec4{ abb[0].x, abb[0].y, abb[0].z, 1.0f/*Don't care*/ };
		abb[1] = transform * glm::vec4{ abb[1].x, abb[1].y, abb[1].z, 1.0f/*Don't care*/ };

		//Now the bounding box is not alligned to the world axis (aka is OBB)
		//	so we need to make ABB again
		let temp = abb[0];
		abb[0] = glm::min(temp, abb[1]);
		abb[1] = glm::max(temp, abb[1]);

		sRendererData.Meshes.emplace_back(node);
	}

	void Renderer3D::SetSkybox(const GPU::Texture* Equirectangular, glm::ivec2 size)
	{
		constexpr glm::vec3 zero = { 0.0f, 0.0f, 0.0f };
		let projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 views[] =
		{
			glm::lookAt(zero, {  1.0f,  0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }),
			glm::lookAt(zero, { -1.0f,  0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }),
			glm::lookAt(zero, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }),
			glm::lookAt(zero, {  0.0f, -1.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }),
			glm::lookAt(zero, {  0.0f,  0.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }),
			glm::lookAt(zero, {  0.0f,  0.0f, -1.0f }, {  0.0f, -1.0f,  0.0f })
		};

		sRendererData.EquirectangularToCubemap->Bind();
		sRendererData.EquirectangularToCubemap->SetUniform("EquirectangularMap", 0);

		auto* fbo = internal::GetContext()->EnvironmentFBO;
		fbo->Resize(size.x, size.y);
		RenderCommand::SetViewport(0, 0, size.x, size.y);
		fbo->Bind();
		Equirectangular->Bind(0);
		for (uint32 i = 0; i <6; i++)
		{
			sRendererData.EquirectangularToCubemap->SetUniform("u_EyeMatrix", projection * views[i]);
			fbo->SpecifyTarget(0, i);
			RenderCommand::Clear();
			
			sRendererData.CubeVAO->Bind();
			RenderCommand::DrawArrays(sRendererData.CubeVAO, 0, 36);
		}
		fbo->Unbind();
		sRendererData.EquirectangularToCubemap->Unbind();
	}

	void Renderer3D::BeginScene(const SceneData& data)
	{
		sRendererData.Meshes.clear();
		sRendererData.Data = data;
		sRendererData.ABB[0] = glm::vec3{ FLT_MAX };
		sRendererData.ABB[1] = glm::vec3{ FLT_MIN };
	}

	void Renderer3D::EndScene(void)
	{
		RenderGeometry();
		RenderSkybox();
	}

	void Renderer3D::RenderSkybox(void)
	{
		sRendererData.SkyboxShader->Bind();
		sRendererData.Data.Target->Bind();
		let view = glm::mat4(glm::mat3(sRendererData.Data.ViewMatrix));
		let eyematrix = sRendererData.Data.ProjectionMatrix * view;
		
		sRendererData.SkyboxShader->SetUniform("u_EyeMatrix", eyematrix);
		sRendererData.SkyboxShader->SetUniform("EnvironmentMap", 0);
		
		let* fbo = internal::GetContext()->EnvironmentFBO;
		fbo->BindAttachment(0, 0);

		sRendererData.CubeVAO->Bind();
		RenderCommand::DrawArrays(sRendererData.CubeVAO, 0, 36);
		sRendererData.Data.Target->Unbind();
		sRendererData.SkyboxShader->Unbind();
	}

	void Renderer3D::Shutdown(void)
	{
		delete sRendererData.ThumbnailShader;
	}

}