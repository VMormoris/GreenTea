#include "Player.h"

using namespace GTE;

void Player::onCollisionStart(ScriptableEntity other)
{
	auto& Tag = other.GetComponent<TagComponent>().Tag;
	if (Tag.compare("Ground") == 0)
	{
		const auto& rb = GetComponent<RigidBody2DComponent>();
		auto& tc = GetComponent<Transform2DComponent>();
		auto& sprite = GetComponent<Renderable2DComponent>();
		if (rb.Velocity.x == 0.0f)
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/IdleSpritesheet.png");
			if (m_CurrentAnimation == &m_JumpLeft)
				m_CurrentAnimation = &m_IdleLeft;
			else if (m_CurrentAnimation == &m_JumpRight)
				m_CurrentAnimation = &m_IdleRight;
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.0f;
		}
		else
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");
			if (m_CurrentAnimation == &m_JumpLeft)
				m_CurrentAnimation = &m_RunLeft;
			else if (m_CurrentAnimation == &m_JumpRight)
				m_CurrentAnimation = &m_RunRight;
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.5f;
		}
		m_OnAir = false;
	}
}

void Player::FixedUpdate(void)
{
	auto& rb = GetComponent<RigidBody2DComponent>();
	auto& sprite = GetComponent<Renderable2DComponent>();
	auto& tc = GetComponent<Transform2DComponent>();

	if (m_JumpTrigger)
	{
		rb.Velocity = glm::vec2(0.0f, 10.0f);
		m_JumpTrigger = false;
		m_OnAir = true;
		if ((m_CurrentAnimation == &m_IdleLeft) || (m_CurrentAnimation == &m_RunLeft))
			m_CurrentAnimation = &m_JumpLeft;
		else
			m_CurrentAnimation = &m_JumpRight;
		m_CurrentAnimation->Reset();
		sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/JumpSpritesheet.png");
		tc.Scale.x = 1.5f;
	}
	else
	{
		if (Input::KeyPressed(KeyCode::D) && Input::KeyPressed(KeyCode::A))
			rb.Velocity.x = 0.0f;
		else if (Input::KeyPressed(KeyCode::D))
			rb.Velocity.x = 8.0f;
		else if (Input::KeyPressed(KeyCode::A))
			rb.Velocity.x = -8.0f;
		else
			rb.Velocity.x = 0.0f;

		if ((rb.Velocity.x > 0.0f) && !m_OnAir && (m_CurrentAnimation != &m_RunRight))
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");
			m_CurrentAnimation = &m_RunRight;
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.5f;
		}
		else if ((rb.Velocity.x > 0.0f) && m_OnAir && (m_CurrentAnimation == &m_JumpLeft))
		{
			m_CurrentAnimation = &m_JumpRight;
			m_CurrentAnimation->Continue(m_JumpLeft);
		}
		else if ((rb.Velocity.x < 0.0f) && !m_OnAir && (m_CurrentAnimation != &m_RunLeft))
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/RunSpritesheet.png");
			m_CurrentAnimation = &m_RunLeft;
			m_CurrentAnimation->Reset();
			tc.Scale.x = 1.5f;
		}
		else if ((rb.Velocity.x < 0.0f) && m_OnAir && (m_CurrentAnimation == &m_JumpRight))
		{
			m_CurrentAnimation = &m_JumpLeft;
			m_CurrentAnimation->Continue(m_JumpRight);
		}
		else if ((rb.Velocity.x == 0.0f) && !m_OnAir && (m_CurrentAnimation != &m_IdleLeft) && (m_CurrentAnimation != &m_IdleRight))
		{
			sprite.Texture = AssetManager::RequestTexture("D:/dev/Sandbox/Assets/Textures/Characters/Ninja/IdleSpritesheet.png");
			if (m_CurrentAnimation == &m_RunLeft)
				m_CurrentAnimation = &m_IdleLeft;
			else if (m_CurrentAnimation == &m_RunRight)
				m_CurrentAnimation = &m_IdleRight;
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
		if ((m_CurrentAnimation != &m_JumpLeft) && (m_CurrentAnimation != &m_JumpRight))
			m_JumpTrigger = m_CurrentAnimation;
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
}