#pragma once

#include <Engine/GPU/Shader.h>
#include <unordered_map>

namespace gte::GPU::OpenGL {

	/**
	* @brief Class for Representing a Shader on OpenGL API
	*/
	class OpenGLShader : public Shader {
	public:

		//Constuctor(s) & Destructor
		OpenGLShader(const char* shader_file) noexcept;
		virtual ~OpenGLShader(void) noexcept;

		void Bind(void) const noexcept override;
		void Unbind(void) const noexcept override;

		void AddUniform(const std::string& uniform) noexcept override;

		bool CreateProgram(void) noexcept override;

		bool ReloadProgram(void) noexcept override;


		//Setters for Uniforms
		void SetUniform(const std::string& uniform, const int32 value) noexcept override;
		void SetUniform(const std::string& uniform, const float value) noexcept override;
		void SetUniform(const std::string& uniform, const glm::vec2& value) noexcept override;
		void SetUniform(const std::string& uniform, const glm::vec3& value) noexcept override;
		void SetUniform(const std::string& uniform, const glm::vec4& value) noexcept override;
		void SetUniform(const std::string& uniform, const glm::mat4& value) noexcept override;
		void SetUniform(const std::string& uniform, const int32* values, uint32 count) noexcept override;

	protected:

		/**
		* Precompile Shader file
		*/
		void PreProcess(std::string& content) noexcept;

	private:

		/**
		* Shader(As String) used for vertices
		*/
		std::string mVertexShader;

		/**
		* Shader(As String) used for fragments
		*/
		std::string mFragmentShader;

		/**
		* Placeholder for each uniform in the Shader
		*/
		std::unordered_map<std::string, int32> mUniforms;

		/**
		* ID used to distinguish each Shader Program in OpenGL
		*/
		uint32 mProgramID;

		/**
		* ID specific to the Vertex Shader in OpenGL
		*/
		uint32 mVsID;

		/**
		* ID specific to the Fragment Shader in OpenGL
		*/
		uint32 mFsID;

	public:

		//ALLOC
		//DEALLOC

	};

}
