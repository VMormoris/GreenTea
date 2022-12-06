
workspace (ProjectName)
	architecture "x64"

	configurations
	{
		"Release",
		"StandAlone",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

IncludeDirs =
{
	(ProjectName .. '/src'),
	(GreenTeaDir .. "/GreenTea/src"),
	(GreenTeaDir .. "/3rdParty/entt/single_include/entt"),
	(GreenTeaDir .. "/3rdParty/glm/glm"),
	(GreenTeaDir .. "/3rdParty/yaml-cpp/include"),
	(GreenTeaDir .. "/3rdParty/IconFontCppHeaders"),
}

LibFiles = { }

require (GreenTeaDir .. "/resources/gt")

project (ProjectName)
	location (ProjectName)
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	
	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/**.h",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.cpp",
	}
	
	includedirs (IncludeDirs)
	links (LibFiles)
	
	disablewarnings {4251}
	
	defines
	{
		"GAME_DLL"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
		}
	
	filter "configurations:Release"
		runtime "Release"
		optimize "on"
	
		links { (GreenTeaDir .. "/bin/" .. outputdir .. "/GreenTea/GreenTea.lib") }
	
		prebuildcommands { gtrDir .. "/bin/" .. outputdir .. "/gtreflect/gtreflect.exe -pre -dir=%{wks.location}" }
		postbuildcommands { gtrDir .. "/bin/" .. outputdir .. "/gtreflect/gtreflect.exe -post -dir=%{wks.location}" }
	
	filter "configurations:StandAlone"
		runtime "Release"
		optimize "on"
		symbols "off"
	
		links { (GreenTeaDir .. "/StandAlone/bin/Dist-windows/GreenTea/GreenTea.lib") }