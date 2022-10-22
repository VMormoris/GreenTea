#include "Renderer2D.h"
#include "RenderCommand.h"

#include <Engine/GPU/VertexArray.h>
#include <Engine/GPU/Shader.h>

#include <array>
#include <gtc/matrix_transform.hpp>

namespace gte {

	struct BufferData {
		glm::vec3 Position;
		glm::vec4 Color;

		union 
		{
			glm::vec2 TextCoord;
			glm::vec2 LocalPosition;
		};
		
		union
		{
			float TextID;
			float Thickness;
		};

		union
		{
			float TilingFactor;
			float Fade;
		};
		
		uint32 ObjectID;
		float IsCircle;
	};

	struct LineData
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct CharData
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextCoord;
		float TextID;
		float ScreenPxRange;
	};


	struct Render2DData {
		const size_t MaxQuads = 20000;
		const size_t MaxVertices = MaxQuads * 4;
		const size_t MaxIndices = MaxQuads * 6;

		static const uint32 MaxTextureSlots = 32;

		GPU::VertexArray* QuadVA = nullptr;
		GPU::VertexBuffer* QuadVB = nullptr;
		GPU::IndexBuffer* QuadIB = nullptr;
		GPU::Shader* Shader2D = nullptr;
		GPU::Texture* WhiteTexture = nullptr;

		uint32 QuadIndexCount = 0;
		BufferData* QuadVertexBufferBase = nullptr;
		BufferData* QuadVertexBufferPtr = nullptr;


		GPU::VertexArray* LineVA = nullptr;
		GPU::VertexBuffer* LineVB = nullptr;
		GPU::Shader* LineShader = nullptr;

		uint32 LineVertexCount = 0;
		LineData* LineVertexBufferBase = nullptr;
		LineData* LineVertexBufferPtr = nullptr;

		float LineThickness = 2.0f;

		GPU::VertexArray* CharVA = nullptr;
		GPU::VertexBuffer* CharVB = nullptr;
		GPU::IndexBuffer* CharIB = nullptr;
		GPU::Shader* TextShader = nullptr;

		uint32 CharIndexCount = 0;
		CharData* CharVertexBufferBase = nullptr;
		CharData* CharVertexBufferPtr = nullptr;

		glm::vec4 QuadVertexPositions[4];
		std::array<const GPU::Texture*, MaxTextureSlots> TextureSlots;
		std::array<const GPU::Texture*, MaxTextureSlots> FontTextureSlots;
		uint32 TextureSlotIndex = 1;
		uint32 FontTextureSlotIndex = 1;
	};

	static Render2DData sData;

	void Renderer2D::Init(void)
	{
		sData.QuadVA = GPU::VertexArray::Create();
		sData.QuadVB = GPU::VertexBuffer::Create(NULL, sData.MaxVertices * sizeof(BufferData));
		sData.QuadVB->SetLayout
		(
			{
				{ GPU::ShaderDataType::Vec3, "_position" },
				{ GPU::ShaderDataType::Vec4, "_color" },
				{ GPU::ShaderDataType::Vec2, "_textCoordsOrLocalPosition" },
				{ GPU::ShaderDataType::Float, "_textIDOrThickness" },
				{ GPU::ShaderDataType::Float, "_TilingFactorOrFade" },
				{ GPU::ShaderDataType::Int, "_objectID"},
				{ GPU::ShaderDataType::Float, "_isCircle"}
			}
		);
		sData.QuadVA->AddVertexBuffer(sData.QuadVB);

		sData.CharVA = GPU::VertexArray::Create();
		sData.CharVB = GPU::VertexBuffer::Create(NULL, sData.MaxVertices * sizeof(BufferData));
		sData.CharVB->SetLayout
		(
			{
				{ GPU::ShaderDataType::Vec3, "_position" },
				{ GPU::ShaderDataType::Vec4, "_color" },
				{ GPU::ShaderDataType::Vec2, "_textCoords" },
				{ GPU::ShaderDataType::Float, "_textID" },
				{ GPU::ShaderDataType::Float, "_screenPxRange" },
			}
		);
		sData.CharVA->AddVertexBuffer(sData.CharVB);

		uint32* quadIndices = new uint32[sData.MaxIndices];
		uint32 offset = 0;
		for (size_t i = 0; i < sData.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		sData.QuadIB = GPU::IndexBuffer::Create(quadIndices, sData.MaxIndices);
		sData.QuadVA->SetIndexBuffer(sData.QuadIB);

		sData.CharIB = GPU::IndexBuffer::Create(quadIndices, sData.MaxIndices);
		sData.CharVA->SetIndexBuffer(sData.CharIB);
		delete[] quadIndices;

		sData.LineVA = GPU::VertexArray::Create();
		sData.LineVB = GPU::VertexBuffer::Create(NULL, sData.MaxVertices * sizeof(LineData));
		sData.LineVB->SetLayout
		(
			{
				{ GPU::ShaderDataType::Vec3, "_position" },
				{ GPU::ShaderDataType::Vec4, "_color" }
			}
		);
		sData.LineVA->AddVertexBuffer(sData.LineVB);

		sData.WhiteTexture = GPU::Texture2D::Create(1, 1);
		uint32 WhiteColor = 0xFFFFFFFF;
		sData.WhiteTexture->SetData(&WhiteColor, sizeof(uint32));

		int32 samplers[sData.MaxTextureSlots];
		for (uint32 i = 0; i < sData.MaxTextureSlots; i++)
			samplers[i] = i;

		sData.Shader2D = GPU::Shader::Create("../Assets/Shaders/Shader2D.glsl");
		sData.Shader2D->Bind();
		sData.Shader2D->AddUniform("u_eyeMatrix");
		sData.Shader2D->AddUniform("u_Textures");
		sData.Shader2D->SetUniform("u_Textures", samplers, sData.MaxTextureSlots);

		sData.TextShader = GPU::Shader::Create("../Assets/Shaders/TextShader.glsl");
		sData.TextShader->Bind();
		sData.TextShader->AddUniform("u_eyeMatrix");
		sData.TextShader->AddUniform("u_Textures");
		sData.TextShader->SetUniform("u_Textures", samplers, sData.MaxTextureSlots);

		sData.LineShader = GPU::Shader::Create("../Assets/Shaders/LineShader.glsl");
		sData.LineShader->Bind();
		sData.LineShader->AddUniform("u_eyeMatrix");

		//First Texture always the white one
		sData.TextureSlots[0] = sData.WhiteTexture;

		sData.QuadVertexPositions[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		sData.QuadVertexPositions[1] = glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);
		sData.QuadVertexPositions[2] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		sData.QuadVertexPositions[3] = glm::vec4(-0.5, 0.5f, 0.0f, 1.0f);

		sData.QuadVertexBufferBase = new BufferData[sData.MaxVertices];
		sData.LineVertexBufferBase = new LineData[sData.MaxVertices];
		sData.CharVertexBufferBase = new CharData[sData.MaxVertices];
	}

	void Renderer2D::BeginScene(const glm::mat4& eyematrix)
	{
		sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
		sData.TextureSlotIndex = 1;
		sData.QuadIndexCount = 0;
		sData.Shader2D->Bind();
		sData.Shader2D->SetUniform("u_eyeMatrix", eyematrix);

		sData.CharVertexBufferPtr = sData.CharVertexBufferBase;
		sData.FontTextureSlotIndex = 0;
		sData.CharIndexCount = 0;
		sData.TextShader->Bind();
		sData.TextShader->SetUniform("u_eyeMatrix", eyematrix);

		sData.LineVertexBufferPtr = sData.LineVertexBufferBase;
		sData.LineVertexCount = 0;
		sData.LineShader->Bind();
		sData.LineShader->SetUniform("u_eyeMatrix", eyematrix);
	}

	void Renderer2D::Flush(void)
	{
		if (sData.QuadIndexCount)
		{
			size_t dataSize = (byte*)sData.QuadVertexBufferPtr - (byte*)sData.QuadVertexBufferBase;//Find size of Buffer to be drawned in bytes
			sData.QuadVB->FillBuffer(sData.QuadVertexBufferBase, dataSize);//Load Buffer into the GPU

			for (uint32 i = 0; i < sData.TextureSlotIndex; i++)// Bind textures to the apropiate slot
				sData.TextureSlots[i]->Bind(i);

			sData.Shader2D->Bind();
			RenderCommand::DrawIndexed(sData.QuadVA, sData.QuadIndexCount);//Draw VertexArray

			//Prepare for a new Batch
			sData.QuadIndexCount = 0;
			sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase;
			sData.TextureSlotIndex = 1;
		}

		if (sData.CharIndexCount)
		{
			size_t dataSize = (byte*)sData.CharVertexBufferPtr - (byte*)sData.CharVertexBufferBase;
			sData.CharVB->FillBuffer(sData.CharVertexBufferBase, dataSize);

			for (uint32 i = 0; i < sData.FontTextureSlotIndex; i++)
				sData.FontTextureSlots[i]->Bind(i);

			sData.TextShader->Bind();
			RenderCommand::DrawIndexed(sData.CharVA, sData.CharIndexCount);

			sData.CharIndexCount = 0;
			sData.CharVertexBufferPtr = sData.CharVertexBufferBase;
			sData.FontTextureSlotIndex = 0;
		}

		if (sData.LineVertexCount)
		{
			size_t dataSize = (byte*)sData.LineVertexBufferPtr - (byte*)sData.LineVertexBufferBase;
			sData.LineVB->FillBuffer(sData.LineVertexBufferBase, dataSize);
			sData.LineShader->Bind();
			RenderCommand::SetLineThickness(sData.LineThickness);
			RenderCommand::DrawLines(sData.LineVA, sData.LineVertexCount);

			sData.LineVertexCount = 0;
			sData.LineVertexBufferPtr = sData.LineVertexBufferBase;
		}
	}

	void Renderer2D::DrawQuad(const glm::mat4& transformation, uint32 ID, const glm::vec4& color, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		constexpr float textureIndex = 0.0f;
		constexpr glm::vec2 textureCoords[quadVertexCount] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		constexpr float circle = 0.0f;

		if (sData.QuadIndexCount >= sData.MaxIndices)
			Flush();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			sData.QuadVertexBufferPtr->Position = transformation * sData.QuadVertexPositions[i];
			sData.QuadVertexBufferPtr->Color = color;
			sData.QuadVertexBufferPtr->TextCoord = textureCoords[i];
			sData.QuadVertexBufferPtr->TextID = textureIndex;
			sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			sData.QuadVertexBufferPtr->ObjectID = ID;
			sData.QuadVertexBufferPtr->IsCircle = circle;
			sData.QuadVertexBufferPtr++;
		}
		sData.QuadIndexCount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, uint32 ID, const glm::vec4& tintColor, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		constexpr float circle = 0.0f;

		if (sData.QuadIndexCount >= sData.MaxIndices)
			Flush();

		float textureIndex = 0.0f;
		for (size_t i = 1; i < sData.TextureSlotIndex; i++)
		{
			if (sData.TextureSlots[i] == texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (sData.TextureSlotIndex >= Render2DData::MaxTextureSlots)
				Flush();
			textureIndex = static_cast<float>(sData.TextureSlotIndex);
			sData.TextureSlots[sData.TextureSlotIndex] = texture;
			sData.TextureSlotIndex++;
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			sData.QuadVertexBufferPtr->Position = transformation * sData.QuadVertexPositions[i];
			sData.QuadVertexBufferPtr->Color = tintColor;
			sData.QuadVertexBufferPtr->TextCoord = textureCoords[i];
			sData.QuadVertexBufferPtr->TextID = textureIndex;
			sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			sData.QuadVertexBufferPtr->ObjectID = ID;
			sData.QuadVertexBufferPtr->IsCircle = circle;
			sData.QuadVertexBufferPtr++;
		}
		sData.QuadIndexCount += 6;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transformation, const GPU::Texture* texture, const TextureCoordinates& textCoords, uint32 ID, const glm::vec4& tintColor, float tilingFactor)
	{
		constexpr size_t quadVertexCount = 4;
		const glm::vec2 textureCoords[] = { textCoords.BottomLeft, textCoords.BottomRight, textCoords.TopRight, textCoords.TopLeft };
		constexpr float circle = 0.0f;

		if (sData.QuadIndexCount >= sData.MaxIndices)
			Flush();

		float textureIndex = 0.0f;
		for (size_t i = 1; i < sData.TextureSlotIndex; i++)
		{
			if (sData.TextureSlots[i] == texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (sData.TextureSlotIndex >= Render2DData::MaxTextureSlots)
				Flush();
			textureIndex = static_cast<float>(sData.TextureSlotIndex);
			sData.TextureSlots[sData.TextureSlotIndex] = texture;
			sData.TextureSlotIndex++;
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			sData.QuadVertexBufferPtr->Position = transformation * sData.QuadVertexPositions[i];
			sData.QuadVertexBufferPtr->Color = tintColor;
			sData.QuadVertexBufferPtr->TextCoord = textureCoords[i];
			sData.QuadVertexBufferPtr->TextID = textureIndex;
			sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			sData.QuadVertexBufferPtr->ObjectID = ID;
			sData.QuadVertexBufferPtr->IsCircle = circle;
			sData.QuadVertexBufferPtr++;
		}
		sData.QuadIndexCount += 6;
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, uint32 ID, float thickness, float fade)
	{
		constexpr size_t quadVertexCount = 4;
		constexpr float circle = 1.0f;

		if (sData.QuadIndexCount >= sData.MaxIndices)
			Flush();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
			sData.QuadVertexBufferPtr->LocalPosition = sData.QuadVertexPositions[i] * 2.0f;
			sData.QuadVertexBufferPtr->Color = color;
			sData.QuadVertexBufferPtr->Thickness = thickness;
			sData.QuadVertexBufferPtr->Fade = fade;
			sData.QuadVertexBufferPtr->ObjectID = ID;
			sData.QuadVertexBufferPtr->IsCircle = circle;
			sData.QuadVertexBufferPtr++;
		}
		sData.QuadIndexCount += 6;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (sData.LineVertexCount > sData.MaxVertices)
			Flush();
		sData.LineVertexBufferPtr->Position = p0;
		sData.LineVertexBufferPtr->Color = color;
		sData.LineVertexBufferPtr++;

		sData.LineVertexBufferPtr->Position = p1;
		sData.LineVertexBufferPtr->Color = color;
		sData.LineVertexBufferPtr++;

		sData.LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * sData.QuadVertexPositions[i];

		DrawLine(lineVertices[0], lineVertices[1], color);
		DrawLine(lineVertices[1], lineVertices[2], color);
		DrawLine(lineVertices[2], lineVertices[3], color);
		DrawLine(lineVertices[3], lineVertices[0], color);
	}

	void Renderer2D::DrawString(const std::string& text, const glm::mat4 transformation, uint32 size, const GPU::Texture* atlas, const internal::Font* font, const glm::vec4& color)
	{
		const float screenPxRange = size / 32.0f * 2.0f;
		if (sData.CharIndexCount >= sData.MaxIndices)
			Flush();
		float textureIndex = -1.0f;
		for (size_t i = 1; i < sData.FontTextureSlotIndex; i++)
		{
			if (sData.FontTextureSlots[i] == atlas)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}

		if (textureIndex == -1.0f)
		{
			if (sData.FontTextureSlotIndex >= Render2DData::MaxTextureSlots)
				Flush();
			textureIndex = static_cast<float>(sData.FontTextureSlotIndex);
			sData.FontTextureSlots[sData.FontTextureSlotIndex] = atlas;
			sData.FontTextureSlotIndex++;
		}

		float cursorPos = 0.0f;
		uint32 prevChar = 0;
		size_t col = 0;
		float line = 0.0f;
		for (const char& c : text)
		{
			uint32 unicode = static_cast<uint32>(c);
			const glm::vec4 uv = font->GetUV(unicode);
			const glm::vec4 localPos = font->GetQuad(unicode, prevChar);
			
			if (c == '\n')
			{
				col = 0;
				cursorPos = 0.0f;
				line++;
			}
			else if (c == '\r')
			{
				col = 0;
				cursorPos = 0.0f;
			}
			else if (c == '\t')
			{
				prevChar = 0;
				uint32 remaining = 4 - (col % 4);
				col += remaining;
				cursorPos += remaining * font->GetAdvance((uint32)' ');
			}
			else
			{
				const std::array<glm::vec2, 4> positions = { glm::vec2{localPos.x, localPos.y - line},  glm::vec2{localPos.z, localPos.y - line}, glm::vec2{localPos.z, localPos.w - line}, glm::vec2{localPos.x, localPos.w - line} };
				const std::array<glm::vec2, 4> coords = { glm::vec2{uv.x, uv.y}, glm::vec2{uv.z, uv.y}, glm::vec2{uv.z, uv.w}, glm::vec2{uv.x, uv.w} };
				for (size_t i = 0; i < 4; i++)
				{
					glm::vec3 pos = glm::vec3{ positions[i], 0.0f };
					pos.x += cursorPos;
					sData.CharVertexBufferPtr->Position = transformation * glm::vec4{ pos, 1.0f };
					sData.CharVertexBufferPtr->Color = color;
					sData.CharVertexBufferPtr->TextCoord = coords[i];
					sData.CharVertexBufferPtr->TextID = textureIndex;
					sData.CharVertexBufferPtr->ScreenPxRange = screenPxRange;
					sData.CharVertexBufferPtr++;
				}

				sData.CharIndexCount += 6;
				prevChar = unicode;
				cursorPos += font->GetAdvance(unicode);
				col++;
			}
			
		}
	}

	void Renderer2D::Shutdown(void)
	{
		delete[] sData.LineVertexBufferBase;
		delete sData.LineShader;
		delete sData.LineVB;
		delete sData.LineVA;

		delete[] sData.CharVertexBufferBase;
		delete sData.TextShader;
		delete sData.CharIB;
		delete sData.CharVB;
		delete sData.CharVA;

		delete[] sData.QuadVertexBufferBase;
		delete sData.Shader2D;
		delete sData.WhiteTexture;
		delete sData.QuadIB;
		delete sData.QuadVB;
		delete sData.QuadVA;
	}

	void Renderer2D::EndScene(void) { Flush(); }

	float Renderer2D::GetLineThickness(void) { return sData.LineThickness; }
	void Renderer2D::SetLineThickness(float thickness) { sData.LineThickness = thickness; }

}