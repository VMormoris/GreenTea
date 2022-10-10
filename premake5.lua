workspace "GreenTea"

	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
	}

	startproject  "CupOfTea"

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

IncludeDirs={}
IncludeDirs["entt"]="3rdParty/entt/single_include/entt"
IncludeDirs["yaml"]="3rdParty/yaml-cpp/include"
IncludeDirs["GLFW"]="3rdParty/glfw/include"
IncludeDirs["Glad"]="3rdParty/glad/include"
IncludeDirs["glm"]="3rdParty/glm/glm"
IncludeDirs["stb"]="3rdParty/stb"
IncludeDirs["imgui"]="3rdParty/imgui"
IncludeDirs["IconHeader"]="3rdParty/IconFontCppHeaders"
IncludeDirs["cereal"]="3rdParty/cereal/include"
IncludeDirs["box2d"]="3rdParty/box2d/include"
IncludeDirs["ImGuizmo"]="3rdParty/ImGuizmo"
IncludeDirs["libsndfile"]="3rdParty/libsndfile/include"
IncludeDirs["openal"]="3rdParty/openal-soft/include"

LibFiles={}
LibFiles["GLFW"]="3rdParty/glfw/libs/SDL2.lib"

include "3rdParty/imgui"
include "3rdParty/box2d"
include "3rdParty/glad"
include "3rdParty/yaml-cpp"
include "3rdParty/glfw"
include "3rdParty/libsndfile"

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
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"3rdParty/imgui/backends/imgui_impl_glfw.h",
		"3rdParty/imgui/backends/imgui_impl_glfw.cpp",
		"3rdParty/imgui/backends/imgui_impl_opengl3.h",
		"3rdParty/imgui/backends/imgui_impl_opengl3.cpp",
	}
	
	disablewarnings {4251, 4275}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.GLFW}",
		"%{IncludeDirs.Glad}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.stb}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.imgui}/backends",
		"%{IncludeDirs.IconHeader}",
		"%{IncludeDirs.box2d}",
		"%{IncludeDirs.openal}",
	}
	
	links
	{
		"ImGui",
		"Box2D",
		"GLFW",
        "yaml-cpp",
		"Glad",
		"opengl32.lib"
	}
		
	defines
	{
		"ENGINE_DLL",
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"AL_LIBTYPE_STATIC",
	}
	
	postbuildcommands
	{
		("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.dll %{wks.location}bin\\" .. outputdir .. "\\CupOfTea"),
		("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.lib %{wks.location}bin\\" .. outputdir .. "\\CupOfTea")
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
		}

		links
		{
			"Winmm.lib",
		}


	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		defines
		{
			"DEBUG_BUILD",
		}

		links
		{
			"3rdParty/openal-soft/Debug/OpenAl32.lib",
		}
		

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		links
        {
            "3rdParty/openal-soft/Release/OpenAl32.lib",
        }
		
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
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/CupOfTea.rc",
		"3rdParty/ImGuizmo/ImGuizmo.h",
		"3rdParty/ImGuizmo/ImGuizmo.cpp",
	}

	disablewarnings {4251, 4275}

	includedirs
	{
		"%{wks.location}/GreenTea/src",
		"%{prj.name}/src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.GLFW}",
		"%{IncludeDirs.Glad}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.IconHeader}",
		"%{IncludeDirs.ImGuizmo}",
		"%{IncludeDirs.libsndfile}",
		"%{IncludeDirs.openal}",
	}

	links
	{
		"GreenTea",
		"ImGui",
		"libsndfile",
		"opengl32.lib",
	}

	debugdir ("%{wks.location}bin\\" .. outputdir .. "\\%{prj.name}")

	--prebuildcommands
	--{
		--("{COPY} %{wks.location}bin\\" .. outputdir .. "\\GreenTea\\**.dll %{wks.location}\\bin\\" .. outputdir .. "\\%{prj.name}"),
		--("{COPY} %{wks.location}bin\\" .. outputdir .. "\\GreenTea\\**.lib %{wks.location}\\bin\\" .. outputdir .. "\\%{prj.name}"),
	--}

	postbuildcommands
	{
		("{COPY} %{wks.location}Assets %{wks.location}bin\\" .. outputdir .. "\\Assets"),
		("{COPY} %{wks.location}resources %{wks.location}bin\\" .. outputdir .. "\\resources"),
		("{COPY} %{wks.location}resources\\imgui.ini %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}"),
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