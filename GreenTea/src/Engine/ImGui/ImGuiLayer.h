#pragma once

#include <Engine/GPU/GraphicsContext.h>

//Forward Declerations
struct ImGuiContext;

namespace gte::gui {

	/**
	* @brief Inteface for creating GUI layers on GreenTea application
	*/
	class ENGINE_API ImGuiLayer {
	public:
		virtual ~ImGuiLayer() = default;
		/**
		*@brief Initialize Gui
		* @param window Pointer to the Native Window
		* @param context Graphics context used for this Window
		*/
		virtual void Init(void* window, GPU::GraphicsContext * context) = 0;

		/**
		* @brief Starts an ImGui Frame
		*/
		virtual void Begin(void) = 0;

		/**
		* @brief Ends an ImGui Frame
		*/
		virtual void End(void) = 0;

		/**
		* @brief Shutdowns the ImGui Layer
		*/
		virtual void Shutdown(void) = 0;

		/**
		* @brief Getter for context
		* returns A pointer to the ImGuiLayer's context
		*/
		[[nodiscard]] virtual ImGuiContext* GetContext(void) noexcept = 0;

		/**
		* @brief Creates a new ImGuiLayer
		* @return A pointer to an ImGuiLayer
		*/
		[[nodiscard]] static ImGuiLayer* Create(void) noexcept;
	};

}