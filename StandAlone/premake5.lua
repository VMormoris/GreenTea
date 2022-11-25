workspace "StandAlone"
	location "."
	architecture "x64"

	configurations
	{
        "Dist",
	}

	startproject  "StandAlone"

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

IncludeDirs={}
IncludeDirs["entt"]="../3rdParty/entt/single_include/entt"
IncludeDirs["yaml"]="../3rdParty/yaml-cpp/include"
IncludeDirs["GLFW"]="../3rdParty/glfw/include"
IncludeDirs["Glad"]="../3rdParty/glad/include"
IncludeDirs["glm"]="../3rdParty/glm/glm"
IncludeDirs["stb"]="../3rdParty/stb"
IncludeDirs["box2d"]="../3rdParty/box2d/include"
IncludeDirs["openal"]="../3rdParty/openal-soft/include"

include "../3rdParty/box2d"
include "../3rdParty/glad"
include "../3rdParty/glfw"
include "../3rdParty/yaml-cpp"
include "../GreenTea/StandAlone.lua"


project "StandAlone"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
	}

    disablewarnings {4251, 4275}

	includedirs
	{
		"../GreenTea/src",
		"src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.GLFW}",
		"%{IncludeDirs.Glad}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.openal}",
	}

	links
	{
		"GreenTea",
		"yaml-cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
	}

	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
		}

		files
		{
			"StandAlone.rc",
		}

		links
		{
			"opengl32.lib",
		}

	filter "system:linux"
		pic "On"
		systemversion "latest"

		libdirs
		{
			"../3rdParty/openal-soft/Release",
		}

		links
		{
			"openal:shared",
			--"opengl32",
		}

	filter "configurations:Dist"
		defines "GT_DIST"
		runtime "Release"
		optimize "on"
		symbols "off"