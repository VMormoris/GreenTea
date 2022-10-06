#pragma once

#include <Engine/Core/Engine.h>
#include <string>
#include <vector>

namespace gte::GPU {

	/**
	* @brief Enumaration for Each valid data type used in the Shaders
	*/
	enum class ENGINE_API ShaderDataType { None = 0, Float, Vec2, Vec3, Vec4, Mat3, Mat4, Int, IVec2, IVec3, IVec4, Boolean };

	/**
	* @brief Converts type to the apropriate size needed for this data type
	*/
	ENGINE_API [[nodiscard]] uint32 sizeofShaderData(ShaderDataType type) noexcept;


	/**
	*@brief Describes a single Element on VertexBuffer
	*/
	struct ENGINE_API BufferElement {
		/**
		* @brief Name of the Element
		*/
		std::string Name;
		/**
		* @brief Offset from the Previous Element
		*/
		uint32 Offset = 0;
		/**
		* @brief Size of the Element
		*/
		uint32 Size = 0;
		/**
		* @brief Data type used for the Element
		*/
		ShaderDataType Type = ShaderDataType::None;
		/**
		* @brief flag to know if a value is normalized
		*/
		bool Normalized = false;

		//Constructors
		BufferElement(void) = default;
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false) noexcept
			: Name(name), Type(type), Size(sizeofShaderData(type)), Offset(0), Normalized(normalized) {}

		/**
		* @returns How many values needed for the Element (ex Vec3 is 3 floats)
		*/
		[[nodiscard]] uint32 Count(void) const noexcept;
	};


	/**
	* @brief Describes how Elements are layed out on the VertexBuffer
	*/
	class ENGINE_API BufferLayout {
	public:

		///Constructors
		BufferLayout(void) = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements) noexcept;

		/**
		* @brief Getter for the stride
		*/
		[[nodiscard]] inline uint32 GetStride(void) const noexcept { return mStride; }

		/**
		* @brief Getter for the Elements of the Layout
		* @return A constant vector containg the Elements
		*/
		[[nodiscard]] inline const std::vector<BufferElement>& GetElements(void) const noexcept { return mElements; }

	private:

		/**
		* @brief Calculates the offsets and the stride
		*/
		void Calculations(void) noexcept;

		/**
		* @brief Placeholder for the Elements of this Layout
		*/
		std::vector<BufferElement> mElements;

		/**
		* @brief Total size need for All Elements
		*/
		uint32 mStride = 0;
	};


	/**
	* @brief Class for Representing a Vertex Buffer
	*/
	class ENGINE_API VertexBuffer {
	public:

		//Destructor
		virtual ~VertexBuffer(void) = default;

		/**
		* @brief Binds the Vertex Buffer
		*/
		virtual void Bind(void) const noexcept = 0;

		/**
		* @brief Unbinds Any currently bound Vertex Buffer
		*/
		virtual void Unbind(void) const noexcept = 0;

		/**
		* @brief Load GPU with the Data for the Vertex Buffer
		* @param vertices Data to be loaded into the GPU
		* @param size How many bytes of data
		*/
		virtual void FillBuffer(const void* vertices, size_t size) noexcept = 0;

		/**
		* @brief Setter for the Buffer's Layout
		*/
		virtual void SetLayout(const BufferLayout& layout) noexcept = 0;
		/**
		* @brief Getter for the Buffer's Layout
		*/
		[[nodiscard]] virtual const BufferLayout& GetLayout(void) const noexcept = 0;

		/**
		* @brief Creates a new VertexBuffer
		* @param vertices Data to be loaded into the GPU
		* @param size How many bytes of data
		* @return A pointer to VertexBuffer object
		*/
		[[nodiscard]] static VertexBuffer* Create(const void* vertices, size_t size) noexcept;

	};

	/**
	* @brief Class for Representing a Index Buffer
	*/
	class ENGINE_API IndexBuffer {
	public:

		//Destructor
		virtual ~IndexBuffer(void) = default;

		/**
		* @brief Binds the Index buffer
		*/
		virtual void Bind(void) const noexcept= 0;

		/**
		* @brief Unbinds any currently bound Index Buffer
		*/
		virtual void Unbind(void) const noexcept = 0;

		/**
		* @brief Load GPU with the indices for the Index Buffer
		* @param indices Data to be loaded into the GPU
		* @param length How many indices
		*/
		virtual void FillBuffer(const uint32* indices, size_t length) noexcept = 0;

		/**
		* @brief How many indices are inside the IndexBuffer
		*/
		[[nodiscard]] virtual size_t GetCount(void) const noexcept = 0;

		/**
		* @brief Creates a new IndexBuffer
		* @param indices Data to be loaded into the GPU
		* @param length How many indices
		* @return A pointer to IndexBuffer object
		*/
		[[nodiscard]] static IndexBuffer* Create(const uint32* indices, size_t length) noexcept;
	};

}
