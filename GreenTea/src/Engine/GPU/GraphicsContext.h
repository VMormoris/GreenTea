#pragma once

#include <Engine/Core/Engine.h>

namespace gte::GPU {

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
		virtual void Init(void* window) noexcept = 0;

		/**
		* @brief Perfoms buffer swapping
		*/
		virtual void SwapBuffers(void) noexcept = 0;

		/**
		* @brieg Creates a GraphicsContext
		* @returns A pointer to GraphicsContext
		*/
		[[nodiscard]] static GraphicsContext* Create(void);

		/**
		* @brieg Setter for the API that will be used for Rendering
		*/
		static void SetAPI(GraphicsAPI API) noexcept;

		/**
		* @brief Getter for the API that is being currently used
		*/
		[[nodiscard]] static const GraphicsAPI GetAPI(void) noexcept;

	};


}