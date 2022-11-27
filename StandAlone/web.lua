workspace "StandAlone"
	location "."
	architecture "x32"

	configurations
	{
        "Web",
	}

	startproject  "StandAlone"

outputdir = "%{cfg.buildcfg}"

IncludeDirs={}
IncludeDirs["entt"]="../3rdParty/entt/single_include/entt"
IncludeDirs["yaml"]="../3rdParty/yaml-cpp/include"
IncludeDirs["GLFW"]="../3rdParty/glfw/include"
IncludeDirs["Glad"]="../3rdParty/glad/include"
IncludeDirs["glm"]="../3rdParty/glm/glm"
IncludeDirs["stb"]="../3rdParty/stb"
IncludeDirs["box2d"]="../3rdParty/box2d/include"

include "../3rdParty/box2d"
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

	includedirs
	{
		"../GreenTea/src",
		"src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.glm}",
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
		"GT_DIST",
		"GT_WEB",
	}

	--buildoptions { "-sERROR_ON_UNDEFINED_SYMBOLS=0" }

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Web"
		runtime "Release"
		optimize "on"
		symbols "off"