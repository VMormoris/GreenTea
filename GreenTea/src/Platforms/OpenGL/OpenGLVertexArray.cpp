#include "OpenGLVertexArray.h"

#include <glad/glad.h>

static [[nodiscard]] GLenum GTEtoOpenGL(gte::GPU::ShaderDataType type) noexcept
{
	switch(type)
	{
		case gte::GPU::ShaderDataType::Float:	return GL_FLOAT;
		case gte::GPU::ShaderDataType::Boolean:	return GL_BOOL;
		case gte::GPU::ShaderDataType::Int:		return GL_INT;
		case gte::GPU::ShaderDataType::Vec2:	return GL_FLOAT;
		case gte::GPU::ShaderDataType::Vec3:	return GL_FLOAT;
		case gte::GPU::ShaderDataType::Vec4:	return GL_FLOAT;
		case gte::GPU::ShaderDataType::Mat3:	return GL_FLOAT;
		case gte::GPU::ShaderDataType::Mat4:	return GL_FLOAT;
		case gte::GPU::ShaderDataType::IVec2:	return GL_INT;
		case gte::GPU::ShaderDataType::IVec3:	return GL_INT;
		case gte::GPU::ShaderDataType::IVec4:	return GL_INT;
	}
	GTE_ERROR_LOG("Unkown Shader Data type!");
	return GL_FLOAT;
}

namespace gte::GPU::OpenGL {


	void OpenGLVertexArray::AddVertexBuffer(const VertexBuffer* vb) noexcept
	{
		Bind();
		vb->Bind();
		const BufferLayout& layout = vb->GetLayout();
		for (const BufferElement& element : layout.GetElements())
		{
			glEnableVertexAttribArray(mVbIndex);
			switch (element.Type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			case ShaderDataType::Vec2:
			case ShaderDataType::Vec3:
			case ShaderDataType::Vec4:
				glVertexAttribPointer
				(
					mVbIndex,
					element.Count(),
					GTEtoOpenGL(element.Type),
					element.Normalized,
					layout.GetStride(),
					(const void*)(uint64)element.Offset
				);
				break;
			case ShaderDataType::Int:
			case ShaderDataType::IVec2:
			case ShaderDataType::IVec3:
			case ShaderDataType::IVec4:
				glVertexAttribIPointer
				(
					mVbIndex,
					element.Count(),
					GTEtoOpenGL(element.Type),
					layout.GetStride(),
					(const void*)(uint64)element.Offset
				);
				break;
			case ShaderDataType::Boolean:
				glVertexAttribIPointer
				(
					mVbIndex,
					element.Count(),
					GTEtoOpenGL(element.Type),
					layout.GetStride(),
					(const void*)(uint64)element.Offset
				);
				break;
			default:
				ENGINE_ASSERT(false, "Not supported ShaderDataType");
			}
			
			mVbIndex++;
		}

		mVertexBuffers.push_back((VertexBuffer*)vb);

	}

	OpenGLVertexArray::OpenGLVertexArray(void) noexcept
	{
		mVbIndex = 0;
		mIndexBuffer = nullptr;
		glGenVertexArrays(1, &mID);
	}

	void OpenGLVertexArray::SetIndexBuffer(const IndexBuffer* ib) noexcept
	{
		Bind();
		ib->Bind();
		mIndexBuffer = (IndexBuffer*)ib;
	}

	void OpenGLVertexArray::Bind(void) const noexcept  { glBindVertexArray(mID); }
	void OpenGLVertexArray::Unbind(void) const noexcept { glBindVertexArray(0); }

	//TODO: Delete Buffer as well?
	OpenGLVertexArray::~OpenGLVertexArray(void) noexcept { glDeleteVertexArrays(1, &mID); }

}