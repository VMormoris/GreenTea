#ifndef _GPU_BUFFER
#define _GPU_BUFFER

#include "GreenTea/Core/EngineCore.h"
#include <string>
#include <vector>

namespace GTE::GPU {

	/**
	* @brief Enumaration for Each valid data type used in the Shaders
	*/
	enum class ENGINE_API ShaderDataType { None = 0, Float, Vec2, Vec3, Vec4, Mat3, Mat4, Int, IVec2, IVec3, IVec4, Boolean };

	/**
	* @brief Converts type to the apropriate size needed for this data type
	*/
	ENGINE_API uint32 sizeofShaderData(ShaderDataType type);


	/**
	*@brief Describes a single Element on VertexBuffer
	*/
	struct ENGINE_API BufferElement {
		/**
		* @brief Name of the Element
		*/
		std::string name;
		/**
		* @brief Offset from the Previous Element
		*/
		uint32 offset = 0;
		/**
		* @brief Size of the Element
		*/
		uint32 size = 0;
		/**
		* @brief Data type used for the Element
		*/
		ShaderDataType type = ShaderDataType::None;
		/**
		* @brief flag to know if a value is normalized
		*/
		bool normalized = false;

		//Constructors
		BufferElement() = default;
		BufferElement(ShaderDataType datatype, const std::string& element_name, bool normal = false) :
			name(element_name), type(datatype), size(sizeofShaderData(datatype)), offset(0), normalized(normal) {}

		/**
		* @returns How many values needed for the Element (ex Vec3 is 3 floats)
		*/
		uint32 Count(void) const;
	};


	/**
	* @brief Describes how Elements are layed out on the VertexBuffer
	*/
	class ENGINE_API BufferLayout {
	public:

		///Constructors
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements);

		/**
		* @brief Getter for the stride
		*/
		inline uint32 GetStride(void) const { return m_Stride; }

		/**
		* @brief Getter for the Elements of the Layout
		* @return A constant vector containg the Elements
		*/
		inline const std::vector<BufferElement>& GetElements(void) const { return m_Elements; }

	private:

		/**
		* @brief Calculates the offsets and the stride
		*/
		void Calculations(void);

		/**
		* @brief Placeholder for the Elements of this Layout
		*/
		std::vector<BufferElement> m_Elements;

		/**
		* @brief Total size need for All Elements
		*/
		uint32 m_Stride = 0;
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
		virtual void Bind(void) const = 0;

		/**
		* @brief Unbinds Any currently bound Vertex Buffer
		*/
		virtual void Unbind(void) const = 0;

		/**
		* @brief Load GPU with the Data for the Vertex Buffer
		* @param vertices Data to be loaded into the GPU
		* @param size How many bytes of data
		*/
		virtual void FillBuffer(const void* vertices, size_t size) = 0;

		/**
		* @brief Setter for the Buffer's Layout
		*/
		virtual void SetLayout(const BufferLayout& layout) = 0;
		/**
		* @brief Getter for the Buffer's Layout
		*/
		virtual const BufferLayout& GetLayout(void) const = 0;

		/**
		* @brief Creates a new VertexBuffer
		* @param vertices Data to be loaded into the GPU
		* @param size How many bytes of data
		* @return A pointer to VertexBuffer object
		*/
		static VertexBuffer* Create(const void* vertices, size_t size);

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
		virtual void Bind(void) const = 0;

		/**
		* @brief Unbinds any currently bound Index Buffer
		*/
		virtual void Unbind(void) const = 0;

		/**
		* @brief Load GPU with the indices for the Index Buffer
		* @param indices Data to be loaded into the GPU
		* @param length How many indices
		*/
		virtual void FillBuffer(const uint32* indices, size_t length) = 0;

		/**
		* @brief How many indices are inside the IndexBuffer
		*/
		virtual size_t GetCount(void) const = 0;

		/**
		* @brief Creates a new IndexBuffer
		* @param indices Data to be loaded into the GPU
		* @param length How many indices
		* @return A pointer to IndexBuffer object
		*/
		static IndexBuffer* Create(const uint32* indices, size_t length);
	};

}
#endif