Includes = { }
LibFiles = { }
workspace (ProjectName)
    architecture "x64"

    configurations
    {
        "Release",
		"StandAlone",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

require (GreenTeaDir .. "/resources/gt")

IncludeDirs =
{
	(ProjectName .. '/src'),
	(GreenTeaDir .. "/GreenTea/src"),
	(GreenTeaDir .. "/3rdParty/entt/single_include/entt"),
	(GreenTeaDir .. "/3rdParty/glm/glm"),
	(GreenTeaDir .. "/3rdParty/yaml-cpp/include"),
	table.unpack(Includes),
}

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

	disablewarnings {4251, 4275}

    defines
	{
		"GAME_DLL"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		exceptionhandling "SEH"

		defines
		{
			"PLATFORM_WINDOWS",
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		links { (GreenTeaDir .. "/bin/" .. outputdir .. "/GreenTea/GreenTea") }

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

	filter "configurations:StandAlone"
		defines { "GT_DIST" }
		runtime "Release"
		optimize "on"
		symbols "off"

		links { (GreenTeaDir .. "/StandAlone/bin/Dist-windows/GreenTea/GreenTea") }
		
		prebuildcommands
		{
			gtrDir .. "/bin/Release-windows/gtreflect/gtreflect.exe -pre -dir=%{wks.location}"
		}
		
		postbuildcommands
		{
			gtrDir .. "/bin/Release-windows/gtreflect/gtreflect.exe -post -dir=%{wks.location}",
		}