#include "HorizontalMovingPlatform.h"

void HorizontalMovingPlatform::Start(void)
{
	auto& rb = GetComponent<RigidBody2DComponent>();
	rb.Velocity.x = 4.0f;
}

void HorizontalMovingPlatform::FixedUpdate(void)
{
	const auto& tc = GetComponent<Transform2DComponent>();
	auto& rb = GetComponent<RigidBody2DComponent>();
	if (tc.Position.x <= -437.0f)
		rb.Velocity.x = 4.0f;
	else if (tc.Position.x >= -415.0f)
		rb.Velocity.x = -4.0f;
}
