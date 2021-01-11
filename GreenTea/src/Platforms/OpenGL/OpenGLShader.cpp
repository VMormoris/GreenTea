#include "OpenGLShader.h"
#include "GreenTea/Core/utils.h"
#include "GreenTea/Core/Logger.h"
#include <GL/glew.h>
#include <gtc/type_ptr.hpp>

static void PrintLog(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else
	{
		GTE_ERROR_LOG("printlog: Not a shader or a program\n");
		return;
	}

	char* log = new char[log_length];

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	GTE_ERROR_LOG(log);
	delete[] log;
}

static uint32 GenerateShader(const char* filename, GLenum shaderType) {

	uint32 res = glCreateShader(shaderType);

	glShaderSource(res, 1, &filename, NULL);

	glCompileShader(res);
	int32 compile_flag = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_flag);

	if (compile_flag == GL_FALSE) {
		GTE_ERROR_LOG("Couldn't Compile: ", filename);
		PrintLog(res);
		return 0;
	}

	return res;

}

namespace GTE::GPU::OpenGL {



	OpenGLShader::OpenGLShader(const char* shader_file) {
		const char* data = utils::readfile(shader_file);
		PreProcess(std::string(data));
		delete[] data;
		CreateProgram();
	}

	bool OpenGLShader::CreateProgram(void) {

		if (m_ProgramID != 0) glDeleteProgram(m_ProgramID);
		m_ProgramID = glCreateProgram();

		if ((m_VsID = GenerateShader(m_VertexShader.c_str(), GL_VERTEX_SHADER)) == 0) return false;
		glAttachShader(m_ProgramID, m_VsID);

		if ((m_FsID = GenerateShader(m_FragmentShader.c_str(), GL_FRAGMENT_SHADER)) == 0) {
			glDeleteShader(m_ProgramID);
			return false;
		}
		glAttachShader(m_ProgramID, m_FsID);

		// link them
		GLint link_ok = GL_FALSE;
		GLint validate_ok = GL_FALSE;
		glLinkProgram(m_ProgramID);
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &link_ok);
		if (!link_ok) {
			GTE_ERROR_LOG("Failed to Link Shaders");
			PrintLog(m_ProgramID);
			glDeleteShader(m_VsID);
			glDeleteShader(m_FsID);
			return false;
		}
		glValidateProgram(m_ProgramID);
		glGetProgramiv(m_ProgramID, GL_VALIDATE_STATUS, &validate_ok);
		if (!validate_ok) {
			GTE_ERROR_LOG("Failed to Validate Shaders");
			glDeleteShader(m_VsID);
			glDeleteShader(m_FsID);
			return false;
		}
		glDeleteShader(m_VsID);
		glDeleteShader(m_FsID);
		return true;

	}

	bool OpenGLShader::ReloadProgram(void)
	{
		if (!CreateProgram()) return false;
		for (auto it : m_Uniforms) {
			it.second = glGetUniformLocation(m_ProgramID, it.first.c_str());
		}
		return true;
	}

	void OpenGLShader::SetUniform(const std::string& uniform, int32 value) { glUniform1i((m_Uniforms[uniform]), value); }
	void OpenGLShader::SetUniform(const std::string& uniform, const float value) { glUniform1f(m_Uniforms[uniform], value); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::vec2& value) { glUniform2f(m_Uniforms[uniform], value[0], value[1]); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::vec3& value) { glUniform3f(m_Uniforms[uniform], value[0], value[1], value[2]); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::vec4& value) { glUniform4f(m_Uniforms[uniform], value[0], value[1], value[2], value[3]); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::mat4& value) { glUniformMatrix4fv(m_Uniforms[uniform], 1, GL_FALSE, glm::value_ptr(value)); }
	void OpenGLShader::SetUniform(const std::string& uniform, const int32* values, uint32 count) { glUniform1iv(m_Uniforms[uniform], count, values); }

	void OpenGLShader::PreProcess(std::string& content) {
		constexpr char* token = "#type";
		size_t token_size = strlen(token);
		size_t pos = content.find(token, 0);
		while (pos != std::string::npos) {
			size_t eol = content.find_first_of("\r\n", pos);
			size_t begin = pos + token_size + 1;
			std::string type = content.substr(begin, eol - begin);

			size_t nextLine = content.find_first_of("\r\n", eol);
			pos = content.find(token, nextLine);

			if (type.compare("vertex") == 0) {
				m_VertexShader = (pos == std::string::npos) ? content.substr(nextLine) : content.substr(nextLine, pos - nextLine);
			}
			else {
				m_FragmentShader = (pos == std::string::npos) ? content.substr(nextLine) : content.substr(nextLine, pos - nextLine);
			}
		}
	}

	void OpenGLShader::Bind(void) const { glUseProgram(m_ProgramID); }
	void OpenGLShader::Unbind(void) const { glUseProgram(0); }
	OpenGLShader::~OpenGLShader(void) { glDeleteProgram(m_ProgramID); }

	void OpenGLShader::AddUniform(const std::string& uniform) { m_Uniforms[uniform] = glGetUniformLocation(m_ProgramID, uniform.c_str()); }
}