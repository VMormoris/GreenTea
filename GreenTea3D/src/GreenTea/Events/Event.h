#ifndef _EVENT
#define _EVENT

#include "Types.h"
#include "Properties.h"

namespace GTE {

	/**
	* @brief Engine's internal Event Representation
	* @see Types.h, Properties.h
	*/
	struct ENGINE_API Event {	
		/**
		* @brief Type of this event
		*/
		EventType type;
		/**
		* @brief Category in which this the event belongs to
		*/
		EventCategory category;
		/**
		* @brief Properties of window during the Event
		*/
		WindowProp window;
		/**
		* @brief Properties of mouse during the Event
		*/
		MouseProp mouse;
		/**
		* @brief Properties of a key during the event
		*/
		KeyboardProp key;
	};
	
	/**
	* @brief Polls a new Event if exists
	* @param event Pointer to an Event to be loaded with the event's data
	* @returns True if event existed, False otherwise
	*/
	bool PollEvent(Event* event);
	
}



#endif