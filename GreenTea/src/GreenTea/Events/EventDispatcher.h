#ifndef _EVENT_DISPATCHER
#define _EVENT_DISPATCHER

#include "Event.h"

namespace GTE {
	
	/**
	* @brief Class for Dispatching Events
	*/
	class ENGINE_API EventDispatcher {
	public:

		/**
		* @brief Dispatches event to the appropriate Listener(s)
		* @details Propagates the events to registered Listeners' callbacks until the event is marked as handled
		* @param event Event that must be dispatched
		*/
		static void Dispatch(const Event& event);

	};

}

#endif