#include "Buffer.h"
#include "GraphicsContext.h"

#include <Platforms/OpenGL/OpenGLBuffer.h>


namespace gte::GPU {

	void BufferLayout::Calculations(void) noexcept
	{
		uint32 offset = 0;
		mStride = 0;
		for (BufferElement& element : mElements) {
			element.Offset = offset;
			offset += element.Size;
			mStride += element.Size;
		}
	}

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) noexcept : mElements(elements) { Calculations(); }

	[[nodiscard]] VertexBuffer* VertexBuffer::Create(const void* data, size_t size) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLVertexBuffer(data, size);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	[[nodiscard]] IndexBuffer* IndexBuffer::Create(const uint32* vertices, size_t length) noexcept
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLIndexBuffer(vertices, length);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	[[nodiscard]] uint32 sizeofShaderData(ShaderDataType type) noexcept
	{
		switch (type)
		{
		case ShaderDataType::Float:			return sizeof(float);
		case ShaderDataType::Int:			return sizeof(int32);
		case ShaderDataType::Boolean:		return sizeof(bool);
		case ShaderDataType::Vec2:			return 2 * sizeof(float);
		case ShaderDataType::Vec3:			return 3 * sizeof(float);
		case ShaderDataType::Vec4:			return 4 * sizeof(float);
		case ShaderDataType::IVec2:			return 2 * sizeof(int32);
		case ShaderDataType::IVec3:			return 3 * sizeof(int32);
		case ShaderDataType::IVec4:			return 4 * sizeof(int32);
		case ShaderDataType::Mat3:			return 3 * 3 * sizeof(float);
		case ShaderDataType::Mat4:			return 4 * 4 * sizeof(float);
		}
		//Unreachable
		GTE_ERROR_LOG("Unkown type is being use!");
		return 0;
	}

	[[nodiscard]] uint32 BufferElement::Count(void) const noexcept
	{
		switch (Type) {
		case ShaderDataType::Float:			return 1;
		case ShaderDataType::Int:			return 1;
		case ShaderDataType::Boolean:		return 1;
		case ShaderDataType::Vec2:			return 2;
		case ShaderDataType::Vec3:			return 3;
		case ShaderDataType::Vec4:			return 4;
		case ShaderDataType::IVec2:			return 2;
		case ShaderDataType::IVec3:			return 3;
		case ShaderDataType::IVec4:			return 4;
		case ShaderDataType::Mat3:			return 3 * 3;
		case ShaderDataType::Mat4:			return 4 * 4;
		}
		//Unreachable
		GTE_ERROR_LOG("Unkown type is being use!");
		return 0;
	}

}