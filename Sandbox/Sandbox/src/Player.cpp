#include "Player.h"

using namespace GTE;

void Player::onCollisionStart(ScriptableEntity other)
{
	auto& Tag = other.GetComponent<TagComponent>().Tag;

	if ((Tag.compare("GravityBoss") == 0) || (Tag.compare("Spikes_0") == 0) || (Tag.compare("Spikes_1") == 0))
		m_DeathTrigger = true;
	if ( IsJumping() && ((Tag.compare("Ground") == 0) || (Tag.compare("HorizontalMovingPlatform") == 0)))
	{
		const auto& rb = GetComponent<RigidBody2DComponent>();
		auto& tc = GetComponent<Transform2DComponent>();
		auto& sprite = GetComponent<Renderable2DComponent>();
		if (rb.Velocity.x == 0.0f)
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/IdleSpritesheet.png");
			if (m_CurrentAnimation == &m_JumpLeft)
			{
				m_AnimationState = AnimationState::IdleLeft;
				m_CurrentAnimation = &m_IdleLeft;
			}
			else if (m_CurrentAnimation == &m_JumpRight)
			{
				m_CurrentAnimation = &m_IdleRight;
				m_AnimationState = AnimationState::IdleRight;
			}
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.0f;
		}
		else
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");
			if (m_CurrentAnimation == &m_JumpLeft)
			{
				m_AnimationState = AnimationState::RunLeft;
				m_CurrentAnimation = &m_RunLeft;
			}
			else if (m_CurrentAnimation == &m_JumpRight)
			{
				m_AnimationState = AnimationState::RunRight;
				m_CurrentAnimation = &m_RunRight;
			}
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.5f;
		}
	}
}

void Player::FixedUpdate(void)
{
	auto& rb = GetComponent<RigidBody2DComponent>();
	auto& sprite = GetComponent<Renderable2DComponent>();
	auto& tc = GetComponent<Transform2DComponent>();

	if (m_DeathTrigger)
	{
		tc.Position = { -500.0f, -7.0f, 0.0f };
		m_DeathTrigger = false;
	}

	if (m_JumpTrigger)
	{
		rb.Velocity = glm::vec2(0.0f, 10.0f);
		m_JumpTrigger = false;
		if ((m_CurrentAnimation == &m_IdleLeft) || (m_CurrentAnimation == &m_RunLeft))
		{
			m_CurrentAnimation = &m_JumpLeft;
			m_AnimationState = AnimationState::JumpLeft;
		}	
		else
		{
			m_AnimationState = AnimationState::JumpRight;
			m_CurrentAnimation = &m_JumpRight;
		}
		m_CurrentAnimation->Reset();
		sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/JumpSpritesheet.png");
		tc.Scale.x = 1.5f;
	}
	else
	{
		if (Input::KeyPressed(KeyCode::D))
		{
			if (IsJumping() && (m_AnimationState == AnimationState::JumpLeft))
			{
				m_AnimationState = AnimationState::JumpRight;
				m_CurrentAnimation = &m_JumpRight;
				m_CurrentAnimation->Continue(m_JumpLeft);
			}
			else if (!IsJumping() && (m_AnimationState != AnimationState::RunRight))
			{
				sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");
				m_AnimationState = AnimationState::RunRight;
				m_CurrentAnimation = &m_RunRight;
				m_CurrentAnimation->Reset();
				tc.Scale.x = 1.5f;
			}
			rb.Velocity.x = 12.0f;
		}
		else if (Input::KeyPressed(KeyCode::A))
		{
			if (IsJumping() && (m_AnimationState == AnimationState::JumpRight))
			{
				m_AnimationState = AnimationState::JumpLeft;
				m_CurrentAnimation = &m_JumpLeft;
				m_CurrentAnimation->Continue(m_JumpRight);
			}
			else if (!IsJumping() && (m_AnimationState != AnimationState::RunLeft))
			{
				sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");
				m_AnimationState = AnimationState::RunLeft;
				m_CurrentAnimation = &m_RunLeft;
				m_CurrentAnimation->Reset();
				tc.Scale.x = 1.5f;
			}
			rb.Velocity.x = -12.0f;
		}
		else if(IsRunning())
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/IdleSpritesheet.png");
			if (m_AnimationState == AnimationState::RunLeft)
			{
				m_AnimationState = AnimationState::IdleLeft;
				m_CurrentAnimation = &m_IdleLeft;
			}
			else if (m_AnimationState == AnimationState::RunRight)
			{
				m_AnimationState = AnimationState::IdleRight;
				m_CurrentAnimation = &m_IdleRight;
			}
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.0f;
		}
	}
}

void Player::Update(float dt)
{
	m_CurrentAnimation->Update(dt);
	auto& sprite = GetComponent<Renderable2DComponent>();
	sprite.TextCoords = m_CurrentAnimation->GetCurrentFrame();
}

bool Player::onKeyDown(GTE::KeyCode keycode)
{
	switch (keycode)
	{
	case KEY_SPACE:
		if (!IsJumping())
			m_JumpTrigger = true;
		return true;
	default:
		return false;
	}
}


void Player::Start(void)
{
	REGISTER(EventType::KeyPressed, this, &Player::onKeyDown);
}

void Player::Destroy(void)
{
	UNREGISTER(this);
}


Player::Player(void)
{
	AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/IdleSpritesheet.png");
	AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/JumpSpritesheet.png");
	AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");

	{
		AnimationSpec spec;
		spec.Width = 1160;
		spec.Height = 878;
		spec.FrameRate = 1.5f / 10.0f;
		spec.TileWidth = 232;
		spec.TileHeight = 439;
		m_IdleRight = Animation(spec);
		m_IdleLeft = m_IdleRight.FlipHorizontally();
	}

	{
		AnimationSpec spec;
		spec.Width = 1448;
		spec.Height = 1449;
		spec.FrameRate = 2.0f / 10.0f;
		spec.TileWidth = 362;
		spec.TileHeight = 483;
		m_JumpRight = Animation(spec);
		m_JumpLeft = m_JumpRight.FlipHorizontally();
	}

	{
		AnimationSpec spec;
		spec.Width = 1452;
		spec.Height = 1374;
		spec.FrameRate = 1.2f / 10.0f;
		spec.TileWidth = 363;
		spec.TileHeight = 458;
		m_RunRight = Animation(spec);
		m_RunLeft = m_RunRight.FlipHorizontally();
	}
	m_CurrentAnimation = &m_IdleRight;
	m_AnimationState = AnimationState::IdleRight;
}

bool Player::IsIdlying(void) const { return m_AnimationState == AnimationState::IdleLeft || m_AnimationState == AnimationState::IdleRight;  }
bool Player::IsRunning(void) const { return m_AnimationState == AnimationState::RunLeft || m_AnimationState == AnimationState::RunRight; }
bool Player::IsJumping(void) const { return m_AnimationState == AnimationState::JumpLeft || m_AnimationState == AnimationState::JumpRight; }