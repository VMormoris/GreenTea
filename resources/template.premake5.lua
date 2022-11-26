
workspace (ProjectName)
    architecture "x64"

    configurations
    {
        "Release"
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

<<<<<<< HEAD
LibFiles = { }
=======
LibFiles =
{
	(GreenTeaDir .. "/bin/" .. outputdir .. "/GreenTea/GreenTea.lib"),
}
>>>>>>> parent of 54e12de (Create export functionality for Windows)

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

	prebuildcommands
	{
		"{ECHO} BuildStarted> %{wks.location}.gt/Notifications",
		gtrDir .. "/bin/" .. outputdir .. "/gtreflect/gtreflect.exe -pre -dir=%{wks.location}"
	}
	postbuildcommands
	{
		gtrDir .. "/bin/" .. outputdir .. "/gtreflect/gtreflect.exe -post -dir=%{wks.location}",
		"{ECHO} BuildEnded> %{wks.location}.gt/Notifications"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
		}		

	filter "system:linux"
		pic "On"
		systemversion "latest"

		libdirs { (GreenTeaDir .. "/StandAlone/bin/Dist-linux/GreenTea") }
		links { "GreenTea" }

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configuration:Dist"
		runtime "Release"
		optimize "on"
		symbols "off"
