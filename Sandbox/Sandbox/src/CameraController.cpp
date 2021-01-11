#include "CameraController.h"

void CameraController::Update(float dt)
{
	auto& tc = GetComponent<Transform2DComponent>();
	Entity ninja = GetEntities("Ninja")[0];
	const auto& ninjaTc = ninja.GetComponent<Transform2DComponent>();
	tc.Position.x = ninjaTc.Position.x;
	tc.Position.y = ninjaTc.Position.y + 7.085f;
}