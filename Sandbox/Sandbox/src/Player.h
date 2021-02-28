#ifndef _PLAYER
#define _PLAYER

#include <GreenTea.h>

#include "Animation.h"



class GAME_API Player : public GTE::ScriptableEntity {
	
	enum class AnimationState {
		//No animation
		None = 0,

		//Idle Animations
		IdleRight = 1,
		IdleLeft = 2,

		//Jump Animations
		JumpRight = 3,
		JumpLeft = 4,

		//Run Animations
		RunLeft = 5,
		RunRight = 6
	};

public:

	Player();

	virtual void Start(void) override;
	virtual void Destroy(void) override;

	virtual void FixedUpdate(void) override;
	virtual void Update(float dt) override;

	virtual void onCollisionStart(ScriptableEntity other) override;

	bool onKeyDown(GTE::KeyCode keycode);

private:

	bool IsJumping(void) const;
	bool IsRunning(void) const;
	bool IsIdlying(void) const;

private:

	bool m_JumpTrigger = false;
	bool m_DeathTrigger = false;

	AnimationState m_AnimationState = AnimationState::None;

	Animation m_IdleRight, m_IdleLeft;
	Animation m_JumpRight, m_JumpLeft;
	Animation m_RunRight, m_RunLeft;
	Animation* m_CurrentAnimation;

};
#endif