#include "Player.h"
#include <gtc/constants.hpp>
#include <gtc/matrix_transform.hpp>

using namespace GTE;



void Player::FixedUpdate(void)
{
}

void Player::Update(float dt)
{
	auto& tc = GetComponent<TransformComponent>();
	auto& persp = GetComponent<PerspectiveCameraComponent>();
	auto& lc = GetComponent<LightComponent>();

	glm::vec3 dir = glm::normalize(persp.Target - tc.Position);
	if (Input::KeyPressed(KeyCode::W))
	{
		tc.Position += dir * m_Velocity * dt;
		persp.Target += dir * m_Velocity * dt;
		lc.Target += dir * m_Velocity * dt;
	}
	if (Input::KeyPressed(KeyCode::S))
	{
		tc.Position -= dir * m_Velocity * dt;
		persp.Target -= dir * m_Velocity * dt;
		lc.Target -= dir * m_Velocity * dt;
	}

	const glm::vec3 right = glm::cross(dir, { 0.0f, 1.0f, 0.0f });

	if (Input::KeyPressed(KeyCode::D))
	{
		tc.Position += right * m_Velocity * dt;
		persp.Target += right * m_Velocity * dt;
		lc.Target += right * m_Velocity * dt;
	}

	if (Input::KeyPressed(KeyCode::A))
	{
		tc.Position -= right * m_Velocity * dt;
		persp.Target -= right * m_Velocity * dt;
		lc.Target -= right * m_Velocity * dt;
	}

	if (m_LookAngleDest != glm::vec2(0.0f))
	{
		const float AngleSpeed = glm::pi<float>() * m_AngularFactor;
		glm::mat4 Rotation = glm::mat4(1.0f);
		m_LookAngleDest *= AngleSpeed;
		GTE_TRACE_LOG("Angle Dest [", m_LookAngleDest.x, ", ", m_LookAngleDest.y, ']');
		Rotation *= glm::rotate(glm::mat4(1.0f), m_LookAngleDest.y, right);
		Rotation *= glm::rotate(glm::mat4(1.0f), m_LookAngleDest.x, persp.UpVector);
		
		dir = Rotation * glm::vec4(dir, 0.0f/*Dont care*/);
		dir = glm::normalize(dir);
		float dist = glm::distance(tc.Position, persp.Target);
		persp.Target += tc.Position + dir * dist;
		dist = glm::distance(tc.Position, lc.Target);
		lc.Target += tc.Position + dir * dist;
	}
}


void Player::Start(void)
{
	//REGISTER(EventType::KeyPressed, this, &Player::onKeyDown);
	REGISTER(EventType::MouseMove, this, &Player::onMouseMove);
}

void Player::Destroy(void)
{
	UNREGISTER(this);
}

bool Player::onMouseMove(int x, int y)
{
	if (Input::MouseButtonPressed(MouseButtonType::Right))
	{
		const glm::vec2 pos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
		if (m_CursorPos == glm::vec2(-1.0f))
			m_LookAngleDest = { 0.0f, 0.0f };
		else
			m_LookAngleDest = glm::vec2(-1.0f, -1.0f) * (pos - m_CursorPos);
		m_CursorPos = pos;
	}
	return true;
}

bool Player::onMouseButtonDown(MouseButtonType btnType)
{
	if (btnType == MouseButtonType::Right)
		m_CursorPos = glm::vec2(-1.0f);
	return true;
}

bool Player::onMouseButtonUp(GTE::MouseButtonType btnType)
{
	if (btnType == MouseButtonType::Right)
		m_LookAngleDest = { 0.0f, 0.0f };
	return true;
}


Player::Player(void)
{

}