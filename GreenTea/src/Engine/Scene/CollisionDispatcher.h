#pragma once

#include "Scene.h"
#include <box2d/b2_world_callbacks.h>

namespace gte {
	/**
		* @brief Singleton For dispatching Collision on events
		*/
	class ENGINE_API CollisionDispatcher : public b2ContactListener {
	public:

		/**
		* @brief Callback for a when a contact between two objects occuries
		* @details The callback will be used by Physics Engine
		*/
		void BeginContact(b2Contact* contact) override;

		/**
		* @brief Callback for a when a contact between two objects stops occuring
		* @details The callback will be used by Physics Engine
		*/
		void EndContact(b2Contact* contact) override;

		static CollisionDispatcher* Get(void);

		//Cannot Copy intances of this object
		CollisionDispatcher(const CollisionDispatcher&) = delete;
		CollisionDispatcher& operator=(const CollisionDispatcher) = delete;

	private:

		//Cannot create intances of this object
		CollisionDispatcher(void) = default;

	};
}