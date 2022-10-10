#pragma once

#include <Engine/Core/Application.h>
#include <Engine/Core/Context.h>
#include <Engine/Core/Random.h>

extern gte::Application* CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	gte::Random::Init();
	auto* context = gte::internal::CreateContext();
	gte::internal::SetContext(context);
	gte::Application* app = CreateApplication(argc, argv);
	app->Run();
	delete app;
	gte::internal::DestroyContext(context);
}