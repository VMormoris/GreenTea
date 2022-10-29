#pragma once

#include <GreenTea.h>

class AnimationPanel {
public:
	AnimationPanel(void);

	void Update(float dt);

	void Draw(bool& show);

	void SetAnimation(const gte::uuid& id);

private:

	bool mHasBeenLoaded = false;
	gte::uuid mAnimationID;
	gte::internal::Animation mAnimation;
	glm::vec2 mViewportSize = { 1.0f, 1.0f };
	gte::GPU::FrameBuffer* mFBO = nullptr;
};