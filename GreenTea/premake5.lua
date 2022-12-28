project "GreenTea"
    kind "SharedLib"
    language "C++"
	cppdialect "C++17"
	
	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	
    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "%{wks.location}/3rdParty/imgui/backends/imgui_impl_glfw.h",
		"%{wks.location}/3rdParty/imgui/backends/imgui_impl_glfw.cpp",
		"%{wks.location}/3rdParty/imgui/backends/imgui_impl_opengl3.h",
		"%{wks.location}/3rdParty/imgui/backends/imgui_impl_opengl3.cpp",
    }

    disablewarnings {4251, 4275}

    includedirs
	{
		"src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.GLFW}",
		"%{IncludeDirs.Glad}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.stb}",
		"%{IncludeDirs.box2d}",
		"%{IncludeDirs.openal}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.imgui}/backends",
		"%{IncludeDirs.IconHeader}",
	}
	
	links
	{
		"Box2D",
		"GLFW",
        "yaml-cpp",
		"Glad",
		"ImGui",
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
		("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.lib %{wks.location}bin\\" .. outputdir .. "\\CupOfTea"),
	}

    filter "system:windows"
		systemversion "latest"

        defines
		{
			"PLATFORM_WINDOWS",
		}

		links
		{
			"Winmm",
			"opengl32",
		}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

        defines { "DEBUG_BUILD" }

		links
		{
			"%{wks.location}/3rdParty/openal-soft/Debug/OpenAl32.lib",
		}
		

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		links
		{
			"%{wks.location}/3rdParty/openal-soft/Release/OpenAl32.lib",
		}