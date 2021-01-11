#ifndef _OPEN_GL_SHADER
#define _OPEN_GL_SHADER

#include "GreenTea/GPU/Shader.h"
#include <unordered_map>

namespace GTE::GPU::OpenGL {

	/**
	* @brief Class for Representing a Shader on OpenGL API
	*/
	class OpenGLShader : public Shader {
	public:

		//Constuctor(s) & Destructor
		OpenGLShader(const char* shader_file);
		virtual ~OpenGLShader(void);

		void Bind(void) const override;
		void Unbind(void) const override;

		void AddUniform(const std::string& uniform) override;

		bool CreateProgram(void) override;

		bool ReloadProgram(void) override;


		//Setters for Uniforms
		void SetUniform(const std::string& uniform, const int32 value) override;
		void SetUniform(const std::string& uniform, const float value) override;
		void SetUniform(const std::string& uniform, const glm::vec2& value) override;
		void SetUniform(const std::string& uniform, const glm::vec3& value) override;
		void SetUniform(const std::string& uniform, const glm::vec4& value) override;
		void SetUniform(const std::string& uniform, const glm::mat4& value) override;
		void SetUniform(const std::string& uniform, const int32* values, uint32 count) override;


	protected:

		/**
		* Precompile Shader file
		*/
		void PreProcess(std::string& content);

		/**
		* Shader(As String) used for vertices
		*/
		std::string m_VertexShader;

		/**
		* Shader(As String) used for fragments
		*/
		std::string m_FragmentShader;

		/**
		* Placeholder for each uniform in the Shader
		*/
		std::unordered_map<std::string, int32> m_Uniforms;

		/**
		* ID used to distinguish each Shader Program in OpenGL
		*/
		uint32 m_ProgramID;

		/**
		* ID specific to the Vertex Shader in OpenGL
		*/
		uint32 m_VsID;

		/**
		* ID specific to the Fragment Shader in OpenGL
		*/
		uint32 m_FsID;

	public:

		//ALLOC
		//DEALLOC

	};

}

#endif