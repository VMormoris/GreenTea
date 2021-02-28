-- Change Sandbox to your project name
ProjectName = "Sandbox"
--Change GreeTeaDir to the absolute path to the GreenTea directory
GreenTeaDir = "D:/dev/GreenTea"

workspace (ProjectName)

	architecture "x64"

	configurations
	{
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDirs={}
IncludeDirs["GreenTea"]=(GreenTeaDir .. "/GreenTea/src")
IncludeDirs["entt"]=(GreenTeaDir .. "/3rdParty/entt/single_include/entt")
IncludeDirs["SDL2"]=(GreenTeaDir .. "/3rdParty/SDL2/includes")
IncludeDirs["Glew"]=(GreenTeaDir .. "/3rdParty/Glew/include")
IncludeDirs["glm"]=(GreenTeaDir .. "/3rdParty/glm/glm")
IncludeDirs["imgui"]=(GreenTeaDir .. "/3rdParty/imgui")
IncludeDirs["box2d"]=(GreenTeaDir .. "/3rdParty/box2d/include")

LibFiles={}
LibFiles["GreenTea"]=(GreenTeaDir .. "/bin/" .. outputdir .. "/GreenTea/GreenTea.lib")
LibFiles["SDL2"]=(GreenTeaDir .. "/3rdParty/SDL2/libs/SDL2.lib")
LibFiles["SDL2main"]=(GreenTeaDir .. "/3rdParty/SDL2/libs/SDL2main.lib")
LibFiles["Glew"]=(GreenTeaDir .. "/3rdParty/Glew/lib/glew32.lib")
LibFiles["box2d"]=(GreenTeaDir .. "/3rdParty/box2d/include")
LibFiles["OpenGL"]="opengl32.lib"


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
		"%{prj.name}/**.cpp"
	}
	
	disablewarnings {4251}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDirs.GreenTea}",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.SDL2}",
		"%{IncludeDirs.Glew}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.box2d}",
	}
	
	links
	{
		"%{LibFiles.GreenTea}",
		"%{LibFiles.SDL2}",
		"%{LibFiles.SDL2main}",
		"%{LibFiles.Glew}",
		"%{LibFiles.OpenGL}",
	}
	
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


	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		defines
		{
			"DEBUG_BUILD"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
