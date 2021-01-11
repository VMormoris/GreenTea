#ifndef _IMGUI_LAYER
#define _IMGUI_LAYER

#include "GreenTea/GPU/GraphicsContext.h"
#include <imgui.h>

namespace GTE {

	/**
	* @brief Layer for ImGui Windows (UI of the Engine)
	*/
	class ENGINE_API ImGuiLayer {
	public:
		virtual ~ImGuiLayer() = default;

		/**
		* @brief Initialize Gui
		* @param window Pointer to the Native Window
		* @param context Graphics context used for this Window
		*/
		virtual void Init(void* window, GPU::GraphicsContext* context) = 0;

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
		virtual ImGuiContext* GetContext(void) = 0;

		/**
		* @brief Creates a new ImGuiLayer
		* @return A pointer to an ImGuiLayer
		*/
		static ImGuiLayer* Create(void);

	};

}

#endif