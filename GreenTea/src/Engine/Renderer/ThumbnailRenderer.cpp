#include "ThumbnailRenderer.h"
#include "RenderCommand.h"

#include <Engine/Assets/Material.h>
#include <Engine/Core/Context.h>
#include <Engine/GPU/Shader.h>

#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

namespace gte {

	struct ThumbnailRendererData {
		GPU::Shader* ThumbnailShader = nullptr;
		GPU::FrameBuffer* ThumbnailFBO = nullptr;
	};

	static ThumbnailRendererData sRendererData;

	void ThumbnailRenderer::Init(void)
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
		sRendererData.ThumbnailShader->AddUniform("u_Shininess");

		sRendererData.ThumbnailShader->AddUniform("u_HasAlbedo");
		sRendererData.ThumbnailShader->AddUniform("u_HasNormal");
		sRendererData.ThumbnailShader->AddUniform("u_HasMetallic");
		sRendererData.ThumbnailShader->AddUniform("u_HasEmissive");
		sRendererData.ThumbnailShader->AddUniform("u_HasOcclusion");
		sRendererData.ThumbnailShader->AddUniform("u_HasOpacity");

		sRendererData.ThumbnailShader->AddUniform("u_CameraPos");
		sRendererData.ThumbnailShader->AddUniform("u_CameraDir");

		sRendererData.ThumbnailShader->AddUniform("u_ViewportSize");

		sRendererData.ThumbnailShader->AddUniform("AlbedoTexture");
		sRendererData.ThumbnailShader->AddUniform("NormalTexture");
		sRendererData.ThumbnailShader->AddUniform("MetallicTexture");
		sRendererData.ThumbnailShader->AddUniform("OclussionTexture");
		sRendererData.ThumbnailShader->AddUniform("OpacityTexture");
		sRendererData.ThumbnailShader->AddUniform("EmissiveTexture");
		
		GPU::FrameBufferSpecification spec;
		spec.Attachments = { gte::GPU::TextureFormat::RGBA8, gte::GPU::TextureFormat::Depth };
		spec.Width = spec.Height = 128;
		sRendererData.ThumbnailFBO = GPU::FrameBuffer::Create(spec);
	}

	void ThumbnailRenderer::Render(Geometry* mesh, const std::vector<uuid>& materials, const glm::ivec2& size)
	{
		constexpr float FoV = glm::radians(75.0f);
		constexpr glm::vec3 CameraDir = { 0.0f, 0.0f, -1.0f };

		GPU::VertexArray* vao = GPU::VertexArray::Create();
		GPU::VertexBuffer* verticesVBO = GPU::VertexBuffer::Create(mesh->Vertices.data(), mesh->Vertices.size() * sizeof(glm::vec3));
		verticesVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_position"} });
		vao->AddVertexBuffer(verticesVBO);

		GPU::VertexBuffer* normalsVBO = GPU::VertexBuffer::Create(mesh->Normals.data(), mesh->Normals.size() * sizeof(glm::vec3));
		normalsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_normal"} });
		vao->AddVertexBuffer(normalsVBO);

		GPU::VertexBuffer* uvsVBO = nullptr;
		if (mesh->UVs.size() > 0)
		{
			uvsVBO = GPU::VertexBuffer::Create(mesh->UVs.data(), mesh->UVs.size() * sizeof(glm::vec2));
			uvsVBO->SetLayout({ {GPU::ShaderDataType::Vec2, "_textCoords"} });
			vao->AddVertexBuffer(uvsVBO);
		}

		GPU::VertexBuffer* tanVBO = nullptr;
		if (mesh->Tangents.size() > 0)
		{
			tanVBO = GPU::VertexBuffer::Create(mesh->Tangents.data(), mesh->Tangents.size() * sizeof(glm::vec3));
			tanVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_tangents"} });
			vao->AddVertexBuffer(tanVBO);
		}

		GPU::VertexBuffer* bitanVBO = nullptr;
		if (mesh->Bitangents.size() > 0)
		{
			bitanVBO = GPU::VertexBuffer::Create(mesh->Bitangents.data(), mesh->Bitangents.size() * sizeof(glm::vec3));
			bitanVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_bitangents"} });
			vao->AddVertexBuffer(bitanVBO);
		}

		let& abb = mesh->ABB;
		const float minVal = glm::min(glm::min(abb[0].x, abb[0].y), abb[0].z);
		const float maxVal = glm::max(glm::max(abb[1].x, abb[1].y), abb[1].z);
		let abblength = maxVal - minVal;

		let z = abblength / (2.0f * glm::tan(FoV / 2));
		let CameraPos = glm::vec3{ 0.0f, 0.0, z };
		let Far = z - abb[0].z + 1.0f;
		let projection = glm::perspective(FoV, 1.0f, 0.0001f, Far);
		let view = glm::lookAt(CameraPos, CameraDir, { 0.0f, 1.0f, 0.0f });
		let eye = projection * view;

		sRendererData.ThumbnailShader->Bind();
		//Camera's Uniforms
		sRendererData.ThumbnailShader->SetUniform("u_EyeMatrix", eye);
		sRendererData.ThumbnailShader->SetUniform("u_CameraPos", CameraPos);
		sRendererData.ThumbnailShader->SetUniform("u_CameraDir", CameraDir);

		//Sampler uniforms
		sRendererData.ThumbnailShader->SetUniform("AlbedoTexture", 1);
		sRendererData.ThumbnailShader->SetUniform("NormalTexture", 2);
		sRendererData.ThumbnailShader->SetUniform("MetallicTexture", 3);
		sRendererData.ThumbnailShader->SetUniform("OclussionTexture", 4);
		sRendererData.ThumbnailShader->SetUniform("OpacityTexture", 5);
		sRendererData.ThumbnailShader->SetUniform("EmissiveTexture", 6);

		if (let& spec = sRendererData.ThumbnailFBO->GetSpecification();
			spec.Width != size.x || spec.Height != size.y)
			sRendererData.ThumbnailFBO->Resize(size.x, size.y);
		sRendererData.ThumbnailFBO->Bind();
		RenderCommand::SetViewport(0, 0, size.x, size.y);
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		RenderCommand::Clear();
		sRendererData.ThumbnailShader->SetUniform("u_ViewportSize", glm::vec2(size.x, size.y));

		let transform = glm::translate(glm::mat4{ 1.0f }, { 0.0f, 0.0f, 0.0f });
		let normalMatrix = glm::mat4(glm::transpose(glm::inverse(transform)));
		sRendererData.ThumbnailShader->SetUniform("u_ModelMatrix", transform);
		sRendererData.ThumbnailShader->SetUniform("u_NormalMatrix", normalMatrix);

		let& parts = mesh->Parts;
		for (let& part : parts)
		{
			let& id = materials[part.MaterialIndex];
			Ref<Asset> material = internal::GetContext()->AssetManager.RequestAsset(id);
			while (material->Type != AssetType::MATERIAL) material = internal::GetContext()->AssetManager.RequestAsset(id);

			let* mat = (Material*)material->Data;
			sRendererData.ThumbnailShader->SetUniform("u_Diffuse", mat->Diffuse);
			sRendererData.ThumbnailShader->SetUniform("u_EmitColor", mat->EmitColor);
			sRendererData.ThumbnailShader->SetUniform("u_AmbientColor", mat->AmbientColor);
			sRendererData.ThumbnailShader->SetUniform("u_Metallicness", mat->Metallicness);
			sRendererData.ThumbnailShader->SetUniform("u_Shininess", mat->Shininess);

			Ref<Asset> albedoTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Albedo->ID);
			while (albedoTexture->Type != AssetType::INVALID && albedoTexture->Type != AssetType::TEXTURE) albedoTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Albedo->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasAlbedo", albedoTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* albedo = (GPU::Texture*)albedoTexture->Data)
				albedo->Bind(1);

			Ref<Asset> normalTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Normal->ID);
			while (normalTexture->Type != AssetType::INVALID && normalTexture->Type != AssetType::TEXTURE) normalTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Normal->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasNormal", normalTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* normal = (GPU::Texture*)normalTexture->Data)
				normal->Bind(2);

			Ref<Asset> metallicTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Metallic->ID);
			while (metallicTexture->Type != AssetType::INVALID && metallicTexture->Type != AssetType::TEXTURE) metallicTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Metallic->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasMetallic", metallicTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* metallic = (GPU::Texture*)metallicTexture->Data)
				metallic->Bind(3);

			Ref<Asset> aoTexture = internal::GetContext()->AssetManager.RequestAsset(mat->AmbientOclussion->ID);
			while (aoTexture->Type != AssetType::INVALID && aoTexture->Type != AssetType::TEXTURE) aoTexture = internal::GetContext()->AssetManager.RequestAsset(mat->AmbientOclussion->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasOcclusion", aoTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* ao = (GPU::Texture*)aoTexture->Data)
				ao->Bind(4);

			Ref<Asset> opacityTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Opacity->ID);
			while (opacityTexture->Type != AssetType::INVALID && opacityTexture->Type != AssetType::TEXTURE) opacityTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Opacity->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasOpacity", opacityTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* opacity = (GPU::Texture*)opacityTexture->Data)
				opacity->Bind(5);

			Ref<Asset> emissiveTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Emission->ID);
			while (emissiveTexture->Type != AssetType::INVALID && emissiveTexture->Type != AssetType::TEXTURE) emissiveTexture = internal::GetContext()->AssetManager.RequestAsset(mat->Emission->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasEmissive", emissiveTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* emmisive = (GPU::Texture*)emissiveTexture->Data)
				emmisive->Bind(6);

			let count = part.End - part.Start;
			RenderCommand::DrawArrays(vao, part.Start, count);
		}
		sRendererData.ThumbnailFBO->Unbind();

		if (bitanVBO) delete bitanVBO;
		if (tanVBO) delete tanVBO;
		if (uvsVBO) delete uvsVBO;
		delete normalsVBO;
		delete verticesVBO;
		delete vao;
	}

	void ThumbnailRenderer::Render(Geometry* mesh, const Material& mat, const glm::ivec2& size, const glm::vec3& rotation)
	{
		constexpr float FoV = glm::radians(75.0f);
		constexpr glm::vec3 CameraDir = { 0.0f, 0.0f, -1.0f };
		constexpr glm::vec3 CameraPos = { 0.0f, 0.0f, 1.7f };
		constexpr float Far = 2.8f;

		GPU::VertexArray* vao = GPU::VertexArray::Create();
		GPU::VertexBuffer* verticesVBO = GPU::VertexBuffer::Create(mesh->Vertices.data(), mesh->Vertices.size() * sizeof(glm::vec3));
		verticesVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_position"} });
		vao->AddVertexBuffer(verticesVBO);

		GPU::VertexBuffer* normalsVBO = GPU::VertexBuffer::Create(mesh->Normals.data(), mesh->Normals.size() * sizeof(glm::vec3));
		normalsVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_normal"} });
		vao->AddVertexBuffer(normalsVBO);

		GPU::VertexBuffer* uvsVBO = nullptr;
		if (mesh->UVs.size() > 0)
		{
			uvsVBO = GPU::VertexBuffer::Create(mesh->UVs.data(), mesh->UVs.size() * sizeof(glm::vec2));
			uvsVBO->SetLayout({ {GPU::ShaderDataType::Vec2, "_textCoords"} });
			vao->AddVertexBuffer(uvsVBO);
		}

		GPU::VertexBuffer* tanVBO = nullptr;
		if (mesh->Tangents.size() > 0)
		{
			tanVBO = GPU::VertexBuffer::Create(mesh->Tangents.data(), mesh->Tangents.size() * sizeof(glm::vec3));
			tanVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_tangents"} });
			vao->AddVertexBuffer(tanVBO);
		}

		GPU::VertexBuffer* bitanVBO = nullptr;
		if (mesh->Bitangents.size() > 0)
		{
			bitanVBO = GPU::VertexBuffer::Create(mesh->Bitangents.data(), mesh->Bitangents.size() * sizeof(glm::vec3));
			bitanVBO->SetLayout({ {GPU::ShaderDataType::Vec3, "_bitangents"} });
			vao->AddVertexBuffer(bitanVBO);
		}

		let projection = glm::perspective(FoV, 1.0f, 0.0001f, Far);
		let view = glm::lookAt(CameraPos, CameraDir, { 0.0f, 1.0f, 0.0f });
		let eye = projection * view;

		sRendererData.ThumbnailShader->Bind();
		//Camera's Uniforms
		sRendererData.ThumbnailShader->SetUniform("u_EyeMatrix", eye);
		sRendererData.ThumbnailShader->SetUniform("u_CameraPos", CameraPos);
		sRendererData.ThumbnailShader->SetUniform("u_CameraDir", CameraDir);

		//Sampler uniforms
		sRendererData.ThumbnailShader->SetUniform("AlbedoTexture", 1);
		sRendererData.ThumbnailShader->SetUniform("NormalTexture", 2);
		sRendererData.ThumbnailShader->SetUniform("MetallicTexture", 3);
		sRendererData.ThumbnailShader->SetUniform("OclussionTexture", 4);
		sRendererData.ThumbnailShader->SetUniform("OpacityTexture", 5);
		sRendererData.ThumbnailShader->SetUniform("EmissiveTexture", 6);

		if (let& spec = sRendererData.ThumbnailFBO->GetSpecification();
			spec.Width != size.x || spec.Height != size.y)
			sRendererData.ThumbnailFBO->Resize(size.x, size.y);
		sRendererData.ThumbnailFBO->Bind();
		RenderCommand::SetViewport(0, 0, size.x, size.y);
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		RenderCommand::Clear();
		sRendererData.ThumbnailShader->SetUniform("u_ViewportSize", glm::vec2(size.x, size.y));

		let transform = glm::translate(glm::mat4{ 1.0f }, { 0.0f, 0.0f, 0.0f })
			* glm::toMat4(glm::quat(glm::radians(rotation)));
		let normalMatrix = glm::mat4(glm::transpose(glm::inverse(transform)));
		sRendererData.ThumbnailShader->SetUniform("u_ModelMatrix", transform);
		sRendererData.ThumbnailShader->SetUniform("u_NormalMatrix", normalMatrix);

		let& parts = mesh->Parts;
		for (let& part : parts)
		{
			sRendererData.ThumbnailShader->SetUniform("u_Diffuse", mat.Diffuse);
			sRendererData.ThumbnailShader->SetUniform("u_EmitColor", mat.EmitColor);
			sRendererData.ThumbnailShader->SetUniform("u_AmbientColor", mat.AmbientColor);
			sRendererData.ThumbnailShader->SetUniform("u_Metallicness", mat.Metallicness);
			sRendererData.ThumbnailShader->SetUniform("u_Shininess", mat.Shininess);

			Ref<Asset> albedoTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Albedo->ID);
			while (albedoTexture->Type != AssetType::INVALID && albedoTexture->Type != AssetType::TEXTURE) albedoTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Albedo->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasAlbedo", albedoTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* albedo = (GPU::Texture*)albedoTexture->Data)
				albedo->Bind(1);

			Ref<Asset> normalTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Normal->ID);
			while (normalTexture->Type != AssetType::INVALID && normalTexture->Type != AssetType::TEXTURE) normalTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Normal->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasNormal", normalTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* normal = (GPU::Texture*)normalTexture->Data)
				normal->Bind(2);

			Ref<Asset> metallicTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Metallic->ID);
			while (metallicTexture->Type != AssetType::INVALID && metallicTexture->Type != AssetType::TEXTURE) metallicTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Metallic->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasMetallic", metallicTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* metallic = (GPU::Texture*)metallicTexture->Data)
				metallic->Bind(3);

			Ref<Asset> aoTexture = internal::GetContext()->AssetManager.RequestAsset(mat.AmbientOclussion->ID);
			while (aoTexture->Type != AssetType::INVALID && aoTexture->Type != AssetType::TEXTURE) aoTexture = internal::GetContext()->AssetManager.RequestAsset(mat.AmbientOclussion->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasOcclusion", aoTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* ao = (GPU::Texture*)aoTexture->Data)
				ao->Bind(4);

			Ref<Asset> opacityTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Opacity->ID);
			while (opacityTexture->Type != AssetType::INVALID && opacityTexture->Type != AssetType::TEXTURE) opacityTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Opacity->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasOpacity", opacityTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* opacity = (GPU::Texture*)opacityTexture->Data)
				opacity->Bind(5);

			Ref<Asset> emissiveTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Emission->ID);
			while (emissiveTexture->Type != AssetType::INVALID && emissiveTexture->Type != AssetType::TEXTURE) emissiveTexture = internal::GetContext()->AssetManager.RequestAsset(mat.Emission->ID);
			sRendererData.ThumbnailShader->SetUniform("u_HasEmissive", emissiveTexture->Type == AssetType::TEXTURE);
			if (GPU::Texture* emmisive = (GPU::Texture*)emissiveTexture->Data)
				emmisive->Bind(6);

			let count = part.End - part.Start;
			RenderCommand::DrawArrays(vao, part.Start, count);
		}
		sRendererData.ThumbnailFBO->Unbind();

		if (bitanVBO) delete bitanVBO;
		if (tanVBO) delete tanVBO;
		if (uvsVBO) delete uvsVBO;
		delete normalsVBO;
		delete verticesVBO;
		delete vao;
	}

	void ThumbnailRenderer::Shutdown(void)
	{
		delete sRendererData.ThumbnailFBO;
		delete sRendererData.ThumbnailShader;
	}


	GPU::FrameBuffer* ThumbnailRenderer::GetThumbnail(void) { return sRendererData.ThumbnailFBO; }
}