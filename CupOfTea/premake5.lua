file = io.open("src/vs.h", "w")
io.output(file)
io.write("#pragma once\n")
io.write("#define ACTION \"".. _ACTION .. "\"")	
io.close()

project "CupOfTea"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++17"
	
	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "%{wks.location}/3rdParty/ImGuizmo/ImGuizmo.h",
		"%{wks.location}/3rdParty/ImGuizmo/ImGuizmo.cpp",
        "CupOfTea.rc",
    }

    disablewarnings { 4251, 4275 }

    includedirs
	{
        "src",
		"%{wks.location}/GreenTea/src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.GLFW}",
		"%{IncludeDirs.Glad}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.IconHeader}",
		"%{IncludeDirs.ImGuizmo}",
		"%{IncludeDirs.libsndfile}",
		"%{IncludeDirs.msdfatlasgen}",
		"%{IncludeDirs.msdfgen}",
		"%{IncludeDirs.openal}",
	}

    links
	{
		"GreenTea",
		"ImGui",
		"yaml-cpp",
		"libsndfile",
		"freetype",
		"msdfgen",
		"msdf-atlas-gen",
	}

	debugdir ("%{wks.location}bin\\" .. outputdir .. "\\%{prj.name}")

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

		exceptionhandling "SEH"

        links { "opengl32" }

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
