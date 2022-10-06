#include "ScriptableEntity.h"

namespace gte {

	void ScriptableEntity::Start(void) {}
	void ScriptableEntity::Destroy(void) {}

	void ScriptableEntity::FixedUpdate(void) {}
	void ScriptableEntity::Update(float dt) {}

	void ScriptableEntity::onCollisionStart(Entity other) {}
	void ScriptableEntity::onCollisionStop(Entity other) {}

}