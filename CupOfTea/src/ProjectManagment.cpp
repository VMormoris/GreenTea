#include "ProjectManagment.h"

#include <imgui.h>
#include <IconsForkAwesome.h>

#include <fstream>

static std::vector<std::string> sProjects;

static gte::GPU::Texture* sIcon = nullptr;

static void CreateProject(const std::string& location, const std::string& name) noexcept;
static void OpenProject(const std::string& path) noexcept;
static void CacheProjects(void) noexcept;

//From: https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
template <typename TP>
std::time_t to_time_t(TP tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
		+ system_clock::now());
	time_t time = system_clock::to_time_t(sctp);
	return time;
}

void ProjectManagment::RenderGUI(void)
{
	gui->Begin();
	ImGui::SetCurrentContext(gui->GetContext());

	ImGuiIO& io = ImGui::GetIO();
	auto BigFont = io.Fonts->Fonts[4];
	auto IconsFont = io.Fonts->Fonts[3];

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("CupOfTea(Project Manager)", nullptr, window_flags);
	ImGui::Image(sIcon->GetID(), { 128.0f, 128.0f }, { 0, 1 }, { 1, 0 });
	ImGui::SameLine(0.0f, 16.0f);
	ImGui::SetCursorPosY(56.0f);
	ImGui::PushFont(BigFont);
	ImGui::Text("Projects");
	ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 590.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8.0f, 4.0f });
	if (ImGui::Button("Open Project"))
	{
		std::string dir = gte::internal::PeekDirectory();
		if (!dir.empty())
		{
			auto prjdir = std::filesystem::path(dir);
			std::string path = (prjdir / (prjdir.filename().string() + ".gt")).string();
			std::replace(path.begin(), path.end(), '\\', '/');
			OpenProject(path);
			Close();
		}
	}
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.129f, 0.588f, 0.952f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.229f, 0.688f, 1.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.079f, 0.538f, 0.902f, 1.0f });
	if (ImGui::Button("New Project"))
		ImGui::OpenPopup("New Project##Modal");
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);
	ImGui::PopFont();

	static std::string error = "";
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
	if (ImGui::BeginPopupModal("New Project##Modal", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		static std::string prjname = "";
		char prjbuffer[65];
		memcpy(prjbuffer, prjname.c_str(), prjname.size() + 1);
		ImGui::Text("Project Name:"); ImGui::SameLine();
		if (ImGui::InputText("##Project Name", prjbuffer, 64))
			prjname = std::string(prjbuffer);

		static std::string location = "";
		char locbuffer[1024];
		memcpy(locbuffer, location.c_str(), location.size() + 1);
		ImGui::Text("Location:"); ImGui::SameLine(0.0f, 41.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - 19.0f);
		if (ImGui::InputText("##Location", locbuffer, 1023))
			location = std::string(locbuffer);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		ImGui::PushFont(IconsFont);
		if (ImGui::Button(ICON_FK_FOLDER, { 24.0f, 24.0f }))
		{
			std::string temp = gte::internal::PeekDirectory();
			if (!temp.empty())
			{
				location = temp;
				std::replace(location.begin(), location.end(), '\\', '/');
			}

		}
		ImGui::PopFont();

		ImGui::Dummy({ 0.0f, 0.0f });
		ImGui::SameLine(0.0f, 105.0f);
		if (ImGui::Button("Ok", { 113.0f, 0.0f }))
		{
			bool err = true;
			if (prjname.empty() && location.empty())
				error = "Please specify project's name and location.";
			else if (prjname.empty())
				error = "Please specify project's name.";
			else if (location.empty())
				error = "Please specify location.";
			else if (!std::filesystem::exists(location))
				error = "The specified location doesn't exists.";
			else if (std::filesystem::exists(location + "/" + prjname))
				error = "The is already a folder/project with that name.";
			else
				err = false;
			if (err)
				ImGui::OpenPopup("Error");
			else
			{
				CreateProject(location, prjname);
				ImGui::CloseCurrentPopup();
				Close();
			}
		}
		if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text(error.c_str());
			ImGui::Dummy({ 0.0f, 0.0f });
			ImGui::SameLine(0.0f, 254.0f);
			if (ImGui::Button("Ok", { 60.0f, 0.0f }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", { 112.0f, 0.0f }))
			ImGui::CloseCurrentPopup();
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	static std::string filter = "";
	ImGui::SetCursorPos({ 887.0f, 116.0f });
	gte::gui::DrawSearchbar("Project Search", filter, 64);
	ImGui::Dummy({ 0.0f, 1.0f });


	static float col1, col2, col3;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f });
	ImVec2 childSize = { 0.0f, ImGui::GetContentRegionAvail().y - 28.0f };
	ImGui::BeginChild("Projects Header", childSize);
	ImGui::Columns(3);
	ImGui::Separator();
	ImGui::Text("Name"); col1 = ImGui::GetColumnWidth(); ImGui::NextColumn();
	ImGui::Text("Location"); col2 = ImGui::GetColumnWidth(); ImGui::NextColumn();
	ImGui::Text("Last Modified"); col3 = ImGui::GetColumnWidth(); ImGui::NextColumn();
	ImGui::Columns(1);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
	ImGui::BeginChild("Projects Content");
	ImGui::Separator();
	ImGui::Columns(3);
	ImGui::SetColumnWidth(0, col1);
	ImGui::SetColumnWidth(1, col2);
	ImGui::SetColumnWidth(2, col3);

	static int selected = -1;
	for (size_t i = 0; i < sProjects.size(); i++)
	{
		using namespace std::chrono_literals;
		const auto prjdir = std::filesystem::path(sProjects[i]);
		const auto name = prjdir.filename().string();
		if (!filter.empty())
		{
			std::string temp = "";
			std::for_each(name.begin(), name.end(), [&](const char c) { temp += std::tolower(c); });
			if (temp.find(filter) == std::string::npos)
				continue;
		}
		auto time = to_time_t(std::filesystem::last_write_time(prjdir));

		std::stringstream timestream;
		timestream << std::put_time(std::localtime(&time), "%F %T");

		if (ImGui::Selectable(name.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
			selected = static_cast<int32>(i);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			const auto gtfile = prjdir / (name + ".gt");
			ShellExecuteA(0, 0, gtfile.string().c_str(), 0, 0, SW_SHOW);
			Close();
		}
		ImGui::NextColumn();
		ImGui::Text(prjdir.string().c_str()); ImGui::NextColumn();
		ImGui::Text(timestream.str().c_str()); ImGui::NextColumn();
		ImGui::Separator();
	}

	ImGui::Columns(1);
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::Dummy({ 0.0f, 0.0f });
	ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 120.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	if (ImGui::Button("Exit", { 120.0f, 0.0f }))
		Close();
	ImGui::PopStyleVar();
	ImGui::End();
	ImGui::PopStyleVar(2);
	gui->End();
}

ProjectManagment::ProjectManagment(void)
	: Application({"CupOfTea(Project Manager)", 0, 0, 1080, 720, true, false})
{
	gte::Window* window = gte::internal::GetContext()->GlobalWindow;
	gui = gte::gui::ImGuiLayer::Create();
	gui->Init(window->GetPlatformWindow(), window->GetContext());

	gte::Image img("../Assets/Icons/GreenTeaLogo.png");
	sIcon = gte::GPU::Texture2D::Create(img);

	std::ifstream is("../resources/projects.cache");
	std::string line;
	while (std::getline(is, line))
		sProjects.push_back(line);
}

ProjectManagment::~ProjectManagment(void)
{
	delete sIcon;
	gui->Shutdown();
	delete gui;
}

void ProjectManagment::Update(float dt)
{
	Application::Update(dt);
	RenderGUI();
}

void CreateProject(const std::string& location, const std::string& name) noexcept
{
	const auto prjdir = std::filesystem::path(location) / name;
	auto path = prjdir.string();
	std::replace(path.begin(), path.end(), '\\', '/');
	sProjects.push_back(path);

	//Create directory for source code
	std::filesystem::create_directories(prjdir / name / "src");

	//Copy files
	auto GreenTeaDir = std::filesystem::absolute(std::filesystem::current_path() / "../../..").string();
	std::replace(GreenTeaDir.begin(), GreenTeaDir.end(), '\\', '/');

	std::filesystem::copy("../resources/template.gitignore", prjdir / ".gitignore");
	std::filesystem::copy("../resources/imgui.ini", prjdir / "imgui.ini");

	//Read premake5.lua template
	std::ifstream is("../resources/template.premake5.lua");
	std::stringstream buffer;
	buffer << is.rdbuf();
	is.close();
	//Write premake5.lua
	std::ofstream os(prjdir / "premake5.lua", std::ios::binary);
	os << "ProjectName = \"" << name << "\"\n";
	os << "GreenTeaDir = \"" << GreenTeaDir << "\"\n";
	os << "gtrDir = \"" << GreenTeaDir + "/3rdParty/gtreflect" << "\"\n";
	os << buffer.str();
	os.close();
	
	//Run premake5
	std::string command = std::string("premake5 --file=") + prjdir.string() + "/premake5.lua gt";
	system(command.c_str());
	command = std::string("premake5 --file=") + prjdir.string() + "/premake5.lua vs2019";
	system(command.c_str());

	//Create .gt file and open project using editor
	auto gtfile = prjdir / (name + ".gt");
	os.open(gtfile);
	os.close();
	ShellExecuteA(0, 0, gtfile.string().c_str(), 0, 0, SW_SHOW);

	CacheProjects();
}

void OpenProject(const std::string& path) noexcept
{
	auto GreenTeaDir = std::filesystem::absolute(std::filesystem::current_path() / "../../..").string();
	std::replace(GreenTeaDir.begin(), GreenTeaDir.end(), '\\', '/');

	auto prjdir = std::filesystem::path(path).parent_path();
	const std::string name = prjdir.filename().string();
	sProjects.push_back(path.substr(0, path.find_last_of('/')));

	//Change premake5 file
	const auto premake5 = prjdir.string() + "/premake5.lua";
	std::vector<std::string> lines;
	std::ifstream is("../resources/template.premake5.lua", std::ios::binary);
	std::string line;
	while (std::getline(is, line))
		lines.emplace_back(line);
	is.close();

	std::ofstream os(premake5, std::ios::binary);
<<<<<<< HEAD
	os << "ProjectName = \"" << name << "\"\n";
=======
	os << lines[0];
>>>>>>> parent of 54e12de (Create export functionality for Windows)
	os << "GreenTeaDir = \"" << GreenTeaDir << "\"\n";
	os << "gtrDir = \"" << GreenTeaDir + "/3rdParty/gtreflect" << "\"\n\n";
	os << lines[1] << '\n';
	for (size_t i = 3; i < lines.size(); i++)
		os << lines[i] << '\n';
	os.close();

	//Run premake5
	std::string command = std::string("premake5 --file=") + premake5 + " gt";
	system(command.c_str());
	command = std::string("premake5 --file=") + premake5 + " vs2019";
	system(command.c_str());

	//Open project using editor
	auto gtfile = prjdir / (name + ".gt");
	ShellExecuteA(0, 0, gtfile.string().c_str(), 0, 0, SW_SHOW);

	CacheProjects();
}

void CacheProjects(void) noexcept
{
	std::ofstream os("../resources/projects.cache");
	for (const auto& project : sProjects)
		os << project << '\n';
	os.close();
}