workspace "GreenTea"

	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
	}

	startproject  "CupOfTea"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDirs={}
IncludeDirs["entt"]="3rdParty/entt/single_include/entt"
IncludeDirs["SDL2"]="3rdParty/SDL2/includes"
IncludeDirs["Glew"]="3rdParty/Glew/include"
IncludeDirs["glm"]="3rdParty/glm/glm"
IncludeDirs["stb"]="3rdParty/stb"
IncludeDirs["imgui"]="3rdParty/imgui"
IncludeDirs["IconHeader"]="3rdParty/IconFontCppHeaders"
IncludeDirs["cereal"]="3rdParty/cereal/include"
IncludeDirs["ImGuizmo"]="3rdParty/ImGuizmo"

LibFiles={}
LibFiles["SDL2"]="3rdParty/SDL2/libs/SDL2.lib"
LibFiles["SDL2main"]="3rdParty/SDL2/libs/SDL2main.lib"
LibFiles["Glew"]="3rdParty/Glew/lib/glew32.lib"
LibFiles["OpenGL"]="opengl32.lib"

include "3rdParty/imgui"


project "GreenTea"
	location "GreenTea"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	
	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"3rdParty/imgui/backends/imgui_impl_sdl.h",
		"3rdParty/imgui/backends/imgui_impl_sdl.cpp",
		"3rdParty/imgui/backends/imgui_impl_opengl3.h",
		"3rdParty/imgui/backends/imgui_impl_opengl3.cpp",
	}
	
	disablewarnings {4251}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.SDL2}",
		"%{IncludeDirs.Glew}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.stb}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.imgui}/backends",
		"%{IncludeDirs.IconHeader}",
		"%{IncludeDirs.cereal}",
	}
	
	links
	{
		"%{wks.location}/%{LibFiles.SDL2}",
		"%{wks.location}/%{LibFiles.SDL2main}",
		"%{wks.location}/%{LibFiles.Glew}",
		"%{LibFiles.OpenGL}",
		"ImGui",
	}
		
	defines
	{
		"ENGINE_DLL",
	}
	
	postbuildcommands
	{
		("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.dll %{wks.location}bin\\" .. outputdir .. "\\CupOfTea"),
		--("{COPY} %{wks.location}..\\bin\\" .. outputdir .. "\\%{prj.name}\\**.lib %{wks.location}..\\bin\\" .. outputdir .. "\\GreenTeaEditor")
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
		
project "CupOfTea"
	location "CupOfTea"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"3rdParty/ImGuizmo/ImGuizmo.h",
		"3rdParty/ImGuizmo/ImGuizmo.cpp",
	}
	
	disablewarnings {4251}
	
	includedirs
	{
		"GreenTea/src",
		"Sandbox/src",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.SDL2}",
		"%{IncludeDirs.Glew}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.IconHeader}",
		"%{IncludeDirs.cereal}",
		"%{IncludeDirs.ImGuizmo}",
	}
	
	links
	{
		"GreenTea",
		"%{wks.location}/%{LibFiles.SDL2}",
		"%{wks.location}/%{LibFiles.SDL2main}",
		"%{wks.location}/%{LibFiles.Glew}",
		"%{LibFiles.OpenGL}",
		"ImGui",
	}
	
	
	prebuildcommands
	{
		("{COPY} %{wks.location}3rdParty\\SDL2\\libs\\**.dll %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}"),
		("{COPY} %{wks.location}3rdParty\\glew\\bin\\**.dll %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}"),
	}
	
	postbuildcommands
	{
		("{COPY} %{wks.location}Assets %{wks.location}bin\\" .. outputdir .. "\\Assets"),
		("{COPY} %{wks.location}CupOfTea\\imgui.ini %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}"),
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
		
