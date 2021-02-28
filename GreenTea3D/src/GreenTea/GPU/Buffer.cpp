#include "Buffer.h"
#include "GraphicsContext.h"
#include "GreenTea/Core/Logger.h"

#include "Platforms/OpenGL/OpenGLBuffer.h"


namespace GTE::GPU {

	void BufferLayout::Calculations(void) {
		uint32 offset = 0;
		m_Stride = 0;
		for (BufferElement& element : m_Elements) {
			element.offset = offset;
			offset += element.size;
			m_Stride += element.size;
		}
	}

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements) { Calculations(); }

	VertexBuffer* VertexBuffer::Create(const void* data, size_t size)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLVertexBuffer(data, size);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	IndexBuffer* IndexBuffer::Create(const uint32* vertices, size_t length)
	{
		switch (GraphicsContext::GetAPI())
		{
		case GraphicsAPI::OpenGL: return new OpenGL::OpenGLIndexBuffer(vertices, length);
		default:
			ENGINE_ASSERT(false, "Only OpenGL is currently supported!");
			return nullptr;
		}
	}

	uint32 sizeofShaderData(ShaderDataType type) {
		switch (type)
		{
		case ShaderDataType::Float:			return sizeof(float);
		case ShaderDataType::Int:			return sizeof(int32);
		case ShaderDataType::Boolean:		return sizeof(float);
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

	uint32 BufferElement::Count(void) const
	{
		switch (type) {
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