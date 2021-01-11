#include "OpenGLVertexArray.h"
#include "GreenTea/Core/Logger.h"

#include <GL/glew.h>

static GLenum GTEtoOpenGL(GTE::GPU::ShaderDataType type)
{
	switch(type)
	{
		case GTE::GPU::ShaderDataType::Float:	return GL_FLOAT;
		case GTE::GPU::ShaderDataType::Boolean:	return GL_BOOL;
		case GTE::GPU::ShaderDataType::Int:		return GL_INT;
		case GTE::GPU::ShaderDataType::Vec2:	return GL_FLOAT;
		case GTE::GPU::ShaderDataType::Vec3:	return GL_FLOAT;
		case GTE::GPU::ShaderDataType::Vec4:	return GL_FLOAT;
		case GTE::GPU::ShaderDataType::Mat3:	return GL_FLOAT;
		case GTE::GPU::ShaderDataType::Mat4:	return GL_FLOAT;
		case GTE::GPU::ShaderDataType::IVec2:	return GL_INT;
		case GTE::GPU::ShaderDataType::IVec3:	return GL_INT;
		case GTE::GPU::ShaderDataType::IVec4:	return GL_INT;
	}
	GTE_ERROR_LOG("Unkown Shader Data type!");
	return GL_FLOAT;
}

namespace GTE::GPU::OpenGL {


	void OpenGLVertexArray::AddVertexBuffer(const VertexBuffer* vb)
	{
		Bind();
		vb->Bind();
		const BufferLayout& layout = vb->GetLayout();
		for (const BufferElement& element : layout.GetElements())
		{
			glEnableVertexAttribArray(m_VbIndex);
			switch (element.type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			case ShaderDataType::Vec2:
			case ShaderDataType::Vec3:
			case ShaderDataType::Vec4:
				glVertexAttribPointer
				(
					m_VbIndex,
					element.Count(),
					GTEtoOpenGL(element.type),
					element.normalized,
					layout.GetStride(),
					(const void*)(uint64)element.offset
				);
				break;
			case ShaderDataType::Int:
			case ShaderDataType::IVec2:
			case ShaderDataType::IVec3:
			case ShaderDataType::IVec4:
				glVertexAttribIPointer
				(
					m_VbIndex,
					element.Count(),
					GTEtoOpenGL(element.type),
					layout.GetStride(),
					(const void*)(uint64)element.offset
				);
				break;
			default:
				ENGINE_ASSERT(false, "Not supported ShaderDataType");
			}
			
			m_VbIndex++;
		}

		m_VertexBuffers.push_back((VertexBuffer*)vb);

	}

	OpenGLVertexArray::OpenGLVertexArray(void)
	{
		m_VbIndex = 0;
		m_IndexBuffer = nullptr;
		glGenVertexArrays(1, &m_ID);
	}

	void OpenGLVertexArray::SetIndexBuffer(const IndexBuffer* ib)
	{
		Bind();
		ib->Bind();
		m_IndexBuffer = (IndexBuffer*)ib;
	}

	void OpenGLVertexArray::Bind(void) const { glBindVertexArray(m_ID); }
	void OpenGLVertexArray::Unbind(void) const { glBindVertexArray(0); }

	//TODO: Delete Buffer as well?
	OpenGLVertexArray::~OpenGLVertexArray(void) { glDeleteVertexArrays(1, &m_ID); }

}