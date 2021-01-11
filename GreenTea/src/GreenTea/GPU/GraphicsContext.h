#ifndef _GRAPHICS_CONTEXT
#define _GRAPHICS_CONTEXT

#include "GreenTea/Core/EngineCore.h"

namespace GTE::GPU {

	/**
	* @brief Enumaration for the API renderes Graphics
	*/
	enum class ENGINE_API GraphicsAPI
	{
		/**
		* @brief Not specified API
		*/
		None = 0,
		/**
		* @brief OpenGL API
		*/
		OpenGL = 1,
		/**
		* @brief DirectX11 API
		*/
		DirectX11 = 2,
		/**
		* Vulkan API
		*/
		Vulkan = 3
	};

	/**
	* @brief Graphics' context for the Window
	* @warning Currently GreenTea Application can have only one Window so we can't have two API
	*/
	class ENGINE_API GraphicsContext {
	public:

		virtual ~GraphicsContext(void) = default;

		/**
		* @brief Initialize Context
		* @param window Pointer to a native window
		*/
		virtual void Init(void* window) = 0;

		/**
		* @brief Perfoms buffer swapping
		*/
		virtual void SwapBuffers(void) = 0;

		/**
		* @brieg Creates a GraphicsContext
		* @returns A pointer to GraphicsContext
		*/
		static GraphicsContext* Create(void);

		/**
		* @brieg Setter for the API that will be used for Rendering
		*/
		static void SetAPI(GraphicsAPI API);

		/**
		* @brief Getter for the API that is being currently used
		*/
		static const GraphicsAPI GetAPI(void);

		/**
		* @brief Getter of the actual context
		* @returns A pointer to the actual context casted to void*
		*/
		virtual void* Get(void) = 0;

		/**
		* @brief Getter of the actual context
		* @returns A constant pointer to the actual context casted to const void*
		*/
		virtual const void* Get(void) const = 0;

	private:

		/**
		* @brief The API that is being used currently
		*/
		static GraphicsAPI s_API;
	};


}

#endif