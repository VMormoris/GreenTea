#include "Renderer2D.h"

#include "GreenTea/GPU/VertexArray.h"
#include "GreenTea/GPU/Shader.h"

#include "RenderCommand.h"

#include <array>
#include <gtc/matrix_transform.hpp>

namespace GTE {

	struct QuadVertex {

		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextCoord;
		float TextID;
		uint32 ObjectID;
	};

	struct Renderer2DData {

		const size_t MaxQuads = 10000;
		const size_t MaxVertices = MaxQuads * 4;
		const size_t MaxIndices = MaxQuads * 6;

		static const uint32 MaxTextureSlots = 32;

		GPU::VertexArray* quadVA = nullptr;
		GPU::VertexBuffer* quadVB = nullptr;
		GPU::IndexBuffer* quadIB = nullptr;
		GPU::Shader* shader2D = nullptr;
		GPU::Texture* whiteTexture = nullptr;

		uint32 QuadIndexCount = 0;

		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		glm::vec4 QuadVertexPositions[4];

		std::array<const GPU::Texture*, MaxTextureSlots> textureSlots;
		uint32 textureSlotIndex = 1;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init(void)
	{
		s_Data.quadVA = GPU::VertexArray::Create();

		s_Data.quadVB = GPU::VertexBuffer::Create(NULL, s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.quadVB->SetLayout
		(
			{
				{ GPU::ShaderDataType::Vec3, "_position" },
				{ GPU::ShaderDataType::Vec4, "_color" },
				{ GPU::ShaderDataType::Vec2, "_textCoords" },
				{ GPU::ShaderDataType::Float, "_textID" },
				{ GPU::ShaderDataType::Int, "_objectID"}
			}
		);
		s_Data.quadVA->AddVertexBuffer(s_Data.quadVB);

		uint32* quadIndices = new uint32[s_Data.MaxIndices];
		uint32 offset = 0;
		for (size_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		s_Data.quadIB = GPU::IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.quadVA->SetIndexBuffer(s_Data.quadIB);
		delete[] quadIndices;

		s_Data.whiteTexture = GPU::Texture2D::Create(1, 1);
		uint32 whiteTextureData = 0xFFFFFFFF;
		s_Data.whiteTexture->SetData(&whiteTextureData, sizeof(uint32));

		s_Data.shader2D = GPU::Shader::Create("../Assets/Shaders/Shader2D.glsl");
		s_Data.shader2D->Bind();
		s_Data.shader2D->AddUniform("u_eyeMatrix");
		s_Data.shader2D->AddUniform("u_Textures");

		int32 samplers[s_Data.MaxTextureSlots];
		for (uint32 i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;
		s_Data.shader2D->SetUniform("u_Textures", samplers, s_Data.MaxTextureSlots);

		//Set the first Texture slot to a White Texture
		s_Data.textureSlots[0] = s_Data.whiteTexture;

		s_Data.QuadVertexPositions[0] = glm::vec4(-1.f, -1.f, 0.f, 1.f);
		s_Data.QuadVertexPositions[1] = glm::vec4(1.f, -1.f, 0.f, 1.f);
		s_Data.QuadVertexPositions[2] = glm::vec4(1.f, 1.f, 0.f, 1.f);
		s_Data.QuadVertexPositions[3] = glm::vec4(-1.f, 1.f, 0.f, 1.f);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

	}

	void Renderer2D::BeginScene(const glm::mat4& eyematrix)
	{
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.textureSlotIndex = 1;
		s_Data.QuadIndexCount = 0;

		s_Data.shader2D->Bind();
		s_Data.shader2D->SetUniform("u_eyeMatrix", eyematrix);
	}

	void Renderer2D::Flush(void)
	{
		size_t dataSize = (char*)s_Data.QuadVertexBufferPtr - (char*)s_Data.QuadVertexBufferBase;//Find size of Buffer to be drawned in bytes
		s_Data.quadVB->FillBuffer(s_Data.QuadVertexBufferBase, dataSize);//Load Buffer into the GPU

		for (uint32 i = 0; i < s_Data.textureSlotIndex; i++)// Bind textures to the apropiate slot
			s_Data.textureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data.quadVA, s_Data.QuadIndexCount);//Draw VertexArray

		//Prepare for a new Batch
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.textureSlotIndex = 1;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transformation, uint32 ID, const glm::vec4& color)
	{
		constexpr size_t quadVertexCount = 4;
		constexpr float textureIndex = 0.0f;
		constexpr glm::vec2 textureCoords[quadVertexCount] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices) Flush();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transformation * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TextCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextID = textureIndex;
			s_Data.QuadVertexBufferPtr->ObjectID = ID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

	}

	void Renderer2D::DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, uint32 ID, const glm::vec4& tintColor)
	{
		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			Flush();

		float textureIndex = 0.0f;
		for (size_t i = 1; i < s_Data.textureSlotIndex; i++)
		{
			if (s_Data.textureSlots[i] == texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.textureSlotIndex >= Renderer2DData::MaxTextureSlots)
				Flush();
			textureIndex = (float)s_Data.textureSlotIndex;
			s_Data.textureSlots[s_Data.textureSlotIndex] = texture;
			s_Data.textureSlotIndex++;
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transformation * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TextCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextID = textureIndex;
			s_Data.QuadVertexBufferPtr->ObjectID = ID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;

	}

	void Renderer2D::DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, const TextureCoordinates& textCoords, uint32 ID, const glm::vec4& tintColor)
	{
		constexpr size_t quadVertexCount = 4;
		const glm::vec2 textureCoords[] = { textCoords.BottomLeft, textCoords.BottomRight, textCoords.TopRight, textCoords.TopLeft };

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			Flush();

		float textureIndex = 0.0f;
		for (size_t i = 1; i < s_Data.textureSlotIndex; i++)
		{
			if (s_Data.textureSlots[i] == texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.textureSlotIndex >= Renderer2DData::MaxTextureSlots)
				Flush();
			textureIndex = (float)s_Data.textureSlotIndex;
			s_Data.textureSlots[s_Data.textureSlotIndex] = texture;
			s_Data.textureSlotIndex++;
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transformation * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TextCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextID = textureIndex;
			s_Data.QuadVertexBufferPtr->ObjectID = ID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 6;
	}

	void Renderer2D::Shutdown(void)
	{
		delete s_Data.quadIB;
		delete s_Data.quadVB;
		delete s_Data.quadVA;
		delete s_Data.shader2D;
		delete s_Data.whiteTexture;
		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::EndScene(void) { Flush(); }

}