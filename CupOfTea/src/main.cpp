#include "CupOfTea.h"
#include "ProjectManagment.h"

#define GREEN_TEA_ENTRY_POINT
#include <Engine/Core/EntryPoint.h>

gte::Application* CreateApplication(int argc, char** argv)
{
	if (argc == 1)
		return new ProjectManagment();
	else if (argc == 2)
	{
		std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path());
		return new CupOfTea(argv[1]);
	}
	return nullptr;
}