#ifndef _APPLICATION
#define _APPLICATION

#include "Window.h"

#include <GreenTea/Events/Properties.h>

namespace GTE {

	/**
	* @brief Superclass for every GreenTea Application
	*/
	class ENGINE_API Application {
	public:

		/**
		* @brief Constructs a GreenTea Application
		* @details Default initiliazation for an GreenTea application
		*/
		Application(void);

		/**
		* @brief Constructs a GreenTea Application
		* @details Initiliazation of an GreenTea application with specified window properties
		* @param title Application's window tittle
		* @param width Application's window width
		* @param height Application's window height
		*/
		Application(const char* title, int32 width, int32 height);
		
		/**
		* @brief Destructs a GreenTea Application
		*/
		virtual ~Application(void);

		/**
		* @brief Runs the application
		* @details Responsible for the main loop of a GreenTea Application that runs every frame
		*/
		void Run(void);

		/**
		* @brief Aplication's Updates runs every frame
		*/
		virtual void Update(float dt);

		/**
		* @brief Callback function for WindowResize Events
		*/
		virtual bool onResize(uint32, uint32);

		/**
		* @brief Callback function for Key Press
		*/
		virtual bool onKeyDown(KeyCode keycode);

	protected:

		/**
		* @brief Flag for whether events should be Disptached to the Game
		*/
		bool m_GameEvents = true;

		/**
		* @brief Pointer to Window
		*/
		Window* m_Window;

	private:

		/**
		* @brief Flag for whether the Application is running
		*/
		bool m_Running;

	};


}

#endif