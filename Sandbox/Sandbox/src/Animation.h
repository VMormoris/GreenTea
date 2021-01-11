#pragma once

#include <GreenTea.h>

using namespace GTE;

struct AnimationSpec {
	uint32 Width, Height;
	uint32 TileWidth, TileHeight;
	float FrameRate;
};

class Animation {
public:

	Animation(void) = default;
	Animation(AnimationSpec spec);

	void Reset(void);
	void Update(float dt);
	void Continue(const Animation& other);

	Animation FlipHorizontally(void);

	TextureCoordinates GetCurrentFrame(void);

private:

	std::vector<TextureCoordinates> m_Frames;
	uint64 m_CurrentFrame = 0;
	float m_Accumulator = 0.0f;
	float m_Rate = 0.0f;
};