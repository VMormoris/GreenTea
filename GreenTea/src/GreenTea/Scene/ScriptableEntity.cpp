#include "ScriptableEntity.h"

namespace GTE {

	void ScriptableEntity::Start(void) {}
	void ScriptableEntity::Destroy(void) {}

	void ScriptableEntity::FixedUpdate(void) {}
	void ScriptableEntity::Update(float dt) {}

	void ScriptableEntity::onCollisionStart(ScriptableEntity other) {}
	void ScriptableEntity::onCollisionStop(ScriptableEntity other) {}

}