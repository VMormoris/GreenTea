#include "Animation.h"

Animation::Animation(AnimationSpec spec)
{
	m_Rate = spec.FrameRate;
	uint32 y = spec.Height;
	
	while (y >= spec.TileHeight)
	{
		uint32 x = 0;
		while (x < spec.Width)
		{
			m_Frames.push_back
			(
				{//New TextureCoordinate 
					{(float)x / (float)spec.Width, (float)(y - spec.TileHeight)/(float)spec.Height},//Bottom Left
					{(float)(x + spec.TileWidth)/ (float)spec.Width, (float)(y - spec.TileHeight)/(float)spec.Height},//Bottom Right
					{(float)(x + spec.TileWidth)/(float)spec.Width, (float)y/(float)spec.Height},//Top Right
					{(float)x/(float)spec.Width, (float)y/(float)spec.Height}//Top Left
				}
			);
			if (m_Frames.size() == 10) return;
			x += spec.TileWidth;
		}
		y -= spec.TileHeight;
	}
}

Animation Animation::FlipHorizontally(void)
{
	Animation newone = *this;
	for (auto& frame : newone.m_Frames)
	{
		glm::vec2 temp = frame.BottomLeft;
		frame.BottomLeft = frame.BottomRight;
		frame.BottomRight = temp;

		temp = frame.TopLeft;
		frame.TopLeft = frame.TopRight;
		frame.TopRight = temp;
	}
	return newone;
}

void Animation::Reset(void)
{
	m_Accumulator = 0.0f;
	m_CurrentFrame = 0;
}

void Animation::Continue(const Animation& other)
{
	m_Accumulator = other.m_Accumulator;
	m_CurrentFrame = other.m_CurrentFrame;
}

void Animation::Update(float dt)
{
	m_Accumulator += dt;
	if (m_Accumulator >= m_Rate)
	{
		m_CurrentFrame++;
		m_Accumulator -= m_Rate;
	}
}

TextureCoordinates Animation::GetCurrentFrame(void) { return m_Frames[m_CurrentFrame % m_Frames.size()]; }
