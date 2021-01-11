#include "Types.h"

namespace GTE {

	EventType operator|(EventType left, EventType right)
	{
		return static_cast<EventType>(static_cast<int16>(left) | static_cast<int16>(right));
	}

	EventType operator&(EventType left, EventType right)
	{
		return static_cast<EventType>(static_cast<int16>(left)& static_cast<int16>(right));
	}

}