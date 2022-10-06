#include "OpenGLShader.h"
//#include "GreenTea/Core/utils.h"
#include <glad/glad.h>

#include <gtc/type_ptr.hpp>

#include <fstream>

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

static [[nodiscard]] uint32 GenerateShader(const char* filename, GLenum shaderType) {

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

static [[nodiscard]] const char* readfile(const char* filepath) noexcept;

namespace gte::GPU::OpenGL {

	OpenGLShader::OpenGLShader(const char* shader_file) noexcept {
		const char* data = readfile(shader_file);
		PreProcess(std::string(data));
		delete[] data;
		CreateProgram();
	}

	bool OpenGLShader::CreateProgram(void) noexcept {

		if (mProgramID != 0) glDeleteProgram(mProgramID);
		mProgramID = glCreateProgram();

		if ((mVsID = GenerateShader(mVertexShader.c_str(), GL_VERTEX_SHADER)) == 0) return false;
		glAttachShader(mProgramID, mVsID);

		if ((mFsID = GenerateShader(mFragmentShader.c_str(), GL_FRAGMENT_SHADER)) == 0) {
			glDeleteShader(mProgramID);
			return false;
		}
		glAttachShader(mProgramID, mFsID);

		// link them
		GLint link_ok = GL_FALSE;
		GLint validate_ok = GL_FALSE;
		glLinkProgram(mProgramID);
		glGetProgramiv(mProgramID, GL_LINK_STATUS, &link_ok);
		if (!link_ok) {
			GTE_ERROR_LOG("Failed to Link Shaders");
			PrintLog(mProgramID);
			glDeleteShader(mVsID);
			glDeleteShader(mFsID);
			return false;
		}
		glValidateProgram(mProgramID);
		glGetProgramiv(mProgramID, GL_VALIDATE_STATUS, &validate_ok);
		if (!validate_ok) {
			GTE_ERROR_LOG("Failed to Validate Shaders");
			glDeleteShader(mVsID);
			glDeleteShader(mFsID);
			return false;
		}
		glDeleteShader(mVsID);
		glDeleteShader(mFsID);
		return true;

	}

	bool OpenGLShader::ReloadProgram(void) noexcept
	{
		if (!CreateProgram()) return false;
		for (auto it : mUniforms) {
			it.second = glGetUniformLocation(mProgramID, it.first.c_str());
		}
		return true;
	}

	void OpenGLShader::SetUniform(const std::string& uniform, int32 value) noexcept { glUniform1i((mUniforms[uniform]), value); }
	void OpenGLShader::SetUniform(const std::string& uniform, const float value) noexcept { glUniform1f(mUniforms[uniform], value); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::vec2& value) noexcept { glUniform2f(mUniforms[uniform], value[0], value[1]); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::vec3& value) noexcept { glUniform3f(mUniforms[uniform], value[0], value[1], value[2]); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::vec4& value) noexcept { glUniform4f(mUniforms[uniform], value[0], value[1], value[2], value[3]); }
	void OpenGLShader::SetUniform(const std::string& uniform, const glm::mat4& value) noexcept { glUniformMatrix4fv(mUniforms[uniform], 1, GL_FALSE, glm::value_ptr(value)); }
	void OpenGLShader::SetUniform(const std::string& uniform, const int32* values, uint32 count) noexcept { glUniform1iv(mUniforms[uniform], count, values); }

	void OpenGLShader::PreProcess(std::string& content) noexcept
	{
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
				mVertexShader = (pos == std::string::npos) ? content.substr(nextLine) : content.substr(nextLine, pos - nextLine);
			}
			else {
				mFragmentShader = (pos == std::string::npos) ? content.substr(nextLine) : content.substr(nextLine, pos - nextLine);
			}
		}
	}

	void OpenGLShader::Bind(void) const noexcept { glUseProgram(mProgramID); }
	void OpenGLShader::Unbind(void) const noexcept { glUseProgram(0); }
	OpenGLShader::~OpenGLShader(void) noexcept { glDeleteProgram(mProgramID); }

	void OpenGLShader::AddUniform(const std::string& uniform) noexcept { mUniforms[uniform] = glGetUniformLocation(mProgramID, uniform.c_str()); }
}

static [[nodiscard]] const char* readfile(const char* filepath) noexcept
{
	std::ifstream in(filepath, std::ifstream::ate | std::ifstream::binary);
	if (!in.is_open())
		return nullptr;

	size_t length = in.tellg();
	in.seekg(0, in.beg);

	char* buffer = new char[length + 1];
	in.read(buffer, length);
	buffer[length] = '\0';

	if (!in) {
		delete[]buffer;
		buffer = nullptr;
	}

	in.close();
	return buffer;
}