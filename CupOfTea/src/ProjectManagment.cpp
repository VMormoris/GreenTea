#include "ProjectManagment.h"
#include "vs.h"

#include <imgui.h>
#include <IconsForkAwesome.h>

#include <fstream>
#include <shellapi.h>

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
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 32.0f);
	ImGui::Text("Name"); col1 = ImGui::GetColumnWidth(); ImGui::NextColumn();
	ImGui::Text("Location"); col2 = ImGui::GetColumnWidth(); ImGui::NextColumn();
	ImGui::Text("Last Modified"); col3 = ImGui::GetColumnWidth(); ImGui::NextColumn();
	ImGui::Columns(1);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f });
	ImGui::BeginChild("Projects Content");
	ImGui::Separator();
	ImGui::Columns(5);
	ImGui::SetColumnWidth(0, 32.0f);
	ImGui::SetColumnWidth(1, col1 - 32.0f);
	ImGui::SetColumnWidth(2, col2);
	ImGui::SetColumnWidth(3, col3 - 32.0f);
	ImGui::SetColumnWidth(4, 32.0f);

	static int32 selected = -1;
	static int32 delIndex = -1;
	for (int32 i = 0; i < sProjects.size(); i++)
	{
		using namespace std::chrono_literals;
		const auto prjdir = std::filesystem::path(sProjects[i]);
		const auto name = prjdir.filename().string();
		
		ImGui::PushFont(IconsFont);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 6);
		if (ImGui::Button(ICON_FK_TRASH, { 28.0f, 24.0f }))
		{
			delIndex = i;
			ImGui::OpenPopup("Delete Project##Modal");
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::NextColumn();

		if (!std::filesystem::exists(prjdir) || !std::filesystem::exists(prjdir / (name + ".gt")))
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f); ImGui::Text(name.c_str()); ImGui::NextColumn();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f); ImGui::Text(prjdir.string().c_str()); ImGui::NextColumn();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f); ImGui::Text(""); ImGui::NextColumn();
			
			ImGui::PushFont(IconsFont);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 3.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, { 0.88f, 0.023f, 0.0f, 1.0f });
			ImGui::Text(ICON_FK_EXCLAMATION_CIRCLE);
			ImGui::PopStyleColor();
			ImGui::PopFont();
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(std::filesystem::exists(prjdir) ? "This directory doesn't seem like a GreeTea project." : "Directory doesn't exists.");
				ImGui::EndTooltip();
			}
			ImGui::NextColumn();
			
			ImGui::Separator();
			continue;
		}

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
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
		if (ImGui::Selectable(name.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
			selected = i;
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			const auto gtfile = prjdir / (name + ".gt");
			ShellExecuteA(0, 0, gtfile.string().c_str(), 0, 0, SW_SHOW);
			Close();
		}
		ImGui::NextColumn();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f); ImGui::Text(prjdir.string().c_str()); ImGui::NextColumn();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f); ImGui::Text(timestream.str().c_str()); ImGui::NextColumn();
		ImGui::Text(""); ImGui::NextColumn();
		ImGui::Separator();
	}

	ImGui::Columns(1);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
	if (ImGui::BeginPopupModal("Delete Project##Modal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::TextWrapped("Remove will only delete the reference from the engine. The files will still remain on disk.\nIf you would like to also delete the files on disk please press 'Delete'.\nDeleting projects is inreversable");
		ImGui::Dummy({ 0.0f, 0.0f });
		ImGui::SameLine(0.0f, ImGui::GetContentRegionAvail().x - 190.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.1505f, 0.151f, 1.0f });
		if (ImGui::Button("Cancel"))
		{
			delIndex = -1;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			const auto it = sProjects.begin() + delIndex;
			std::filesystem::remove_all(*it);
			sProjects.erase(it);
			CacheProjects();
			delIndex = -1;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			sProjects.erase(sProjects.begin() + delIndex);
			CacheProjects();
			delIndex = -1;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
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
	sIcon = gte::GPU::Texture2D::Create(img, { gte::GPU::TextureFormat::RGBA8 });

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
	auto GreenTeaDir = gte::internal::GetContext()->GreenTeaDir;

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
	command = std::string("premake5 --file=") + prjdir.string() + "/premake5.lua " ACTION;
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
	auto GreenTeaDir = gte::internal::GetContext()->GreenTeaDir;

	auto prjdir = std::filesystem::path(path).parent_path();
	const std::string name = prjdir.filename().string();
	sProjects.push_back(path.substr(0, path.find_last_of('/')));

	//Change premake5 file
	const auto premake5 = prjdir.string() + "/premake5.lua";
	std::vector<std::string> lines;
	{//Read Includes & Link files
		std::ifstream is(premake5, std::ios::binary);
		std::string line;
		while (std::getline(is, line))
		{
			if (line.find("Includes") != std::string::npos)
			{
				lines.emplace_back(line);
				break;
			}
		}
		while (std::getline(is, line))
		{
			if (line.find("workspace") != std::string::npos)
				break;
			lines.emplace_back(line);
		}
		is.close();
	}

	//Rewrite premake5.lua
	std::ofstream os(premake5, std::ios::binary);
	os << "ProjectName = " << prjdir.stem() << "\n";
	os << "GreenTeaDir = \"" << GreenTeaDir << "\"\n";
	os << "gtrDir = \"" << GreenTeaDir + "/3rdParty/gtreflect" << "\"\n\n";
	for (const auto& line : lines)
		os << line << '\n';
	std::ifstream is("../resources/template.premake5.lua", std::ios::binary);
	std::string line;
	while (std::getline(is, line))
	{
		if (line.compare("Includes = { }\r") == 0 || line.compare("LibFiles = { }\r") == 0)
			continue;
		os << line << '\n';
	}
	os.close();

	//Run premake5
	std::string command = std::string("premake5 --file=") + premake5 + " gt";
	system(command.c_str());
	command = std::string("premake5 --file=") + premake5 + " " ACTION;
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