#include "CupOfTea.h"
#include <filesystem>
#include <fstream>

int main(int argc, char** argv)
{	
	std::ofstream err("error.gtlog");
	Logger::Init(std::cout, err);
	//AssetManager::Init();
	
	Application* app = nullptr;
	if (argc == 2)
	{
		std::string app_filepath(argv[0]);
		size_t pos = app_filepath.find_last_of("/\\");
		std::filesystem::path app_path = app_filepath.substr(0, pos);
		std::filesystem::current_path(app_path);
		app = new CupOfTea(argv[1]);
	}
	else
		app = new CupOfTea();

	app->Run();
	delete app;
	
	AssetManager::Clear();
	err.close();
	return 0;
}
