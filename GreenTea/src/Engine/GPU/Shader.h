#pragma once

#include <Engine/Core/Engine.h>

#include <string>

#include <glm.hpp>

namespace gte::GPU {

	/**
	* @brief Class for representing a Shader
	*/
	class ENGINE_API Shader {
	public:

		//Destructor
		virtual ~Shader(void) = default;

		/**
		* @brief Create a new Shader instance
		* @param vertex_file C-String containing the fullname of the file for the Vertex Shader to load
		* @param fragment_file C-String containing the fullname of the file for the Fragment Shader to load
		* @returns A pointer to Shader object
		*/
		[[nodiscard]] static Shader* Create(const char* shader_file) noexcept;

		/**
		* @brief Binds the Shader
		*/
		virtual void Bind(void) const noexcept = 0;

		/**
		* @brief Unbinds any currently bound Shader
		*/
		virtual void Unbind(void) const noexcept = 0;

		/**
		* @brief Adds Uniform to the Shader
		* @param uniform String that will be used as indetifier for the Uniform
		*/
		virtual void AddUniform(const std::string& uniform) noexcept = 0;


		/**
		* @brief Compiles & Loads Shaders into the GPU
		* @return true if operation succes false otherwise
		*/
		virtual bool CreateProgram(void) noexcept = 0;

		/**
		* @brief Recompiles & Reloads Shaders
		* @details Realy usefull when debugging in order to make changes on the fly
		* @return true if operation succes false otherwise
		*/
		virtual bool ReloadProgram(void) noexcept = 0;


		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param value An integer to be loaded into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const int32 value) noexcept = 0;

		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param value A float to be loaded into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const float value) noexcept = 0;

		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param value A vector of two floats to be loaded into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const glm::vec2& value) noexcept = 0;

		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param value A vector of three float to be loaded into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const glm::vec3& value) noexcept = 0;

		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param value A vector of four floats to be loaded into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const glm::vec4& value) noexcept = 0;

		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param value A 4x4 Matrix to be loaded into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const glm::mat4& value) noexcept = 0;

		/**
		* @brief Setter for a uniform
		* @param uniform Name of the uniform
		* @param values A pointer to an array containing integer values
		* @param count Numbers of elments that will be loaded from the array into the uniform
		*/
		virtual void SetUniform(const std::string& uniform, const int32* values, uint32 count) noexcept = 0;

	};

}
