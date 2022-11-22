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
	}

    links
    {
        "Box2D",
		"GLFW",
        "yaml-cpp",
		"Glad",
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
        ("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.dll %{wks.location}bin\\" .. outputdir .. "\\StandAlone"),
		("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.lib %{wks.location}bin\\" .. outputdir .. "\\StandAlone"),
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
			"opengl32.lib",
			"../3rdParty/openal-soft/Release/OpenAl32.lib",
		}

    filter "configurations:Dist"
        defines "GT_DIST"
        runtime "Release"
        optimize "on"
        symbols "off"