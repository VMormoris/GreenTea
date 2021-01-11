#ifndef _PLAYER
#define _PLAYER

#include <GreenTea.h>

#include "Animation.h"

class GAME_API Player : public GTE::ScriptableEntity {
public:

	Player();

	virtual void Start(void) override;
	virtual void Destroy(void) override;

	virtual void FixedUpdate(void) override;
	virtual void Update(float dt) override;

	virtual void onCollisionStart(ScriptableEntity other) override;

	bool onKeyDown(GTE::KeyCode keycode);

private:

	bool m_JumpTrigger = false;
	bool m_OnAir = false;

	Animation m_IdleRight, m_IdleLeft;
	Animation m_JumpRight, m_JumpLeft;
	Animation m_RunRight, m_RunLeft;
	Animation* m_CurrentAnimation;

};
#endif