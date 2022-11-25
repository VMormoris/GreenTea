project "GreenTea"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
	{
		"src/Engine/**.h",
		"src/Engine/**.hpp",
		"src/Engine/**.cpp",
		"src/Platforms/GLFW/**.h",
		"src/Platforms/GLFW/**.cpp",
		"src/Platforms/OpenGL/**.h",
		"src/Platforms/OpenGL/**.cpp",
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

    filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
		}

		files
		{
			"src/Platforms/Windows/**.h",
			"src/Platforms/Windows/**.hpp",
			"src/Platforms/Windows/**.cpp",
		}

		links
		{
			"Winmm.lib",
			"opengl32.lib",
			"../3rdParty/openal-soft/Release/OpenAl32.lib",
		}

		postbuildcommands
		{
			("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.dll %{wks.location}bin\\" .. outputdir .. "\\StandAlone"),
			("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.lib %{wks.location}bin\\" .. outputdir .. "\\StandAlone"),
		}

	filter "system:linux"
		pic "On"
		systemversion "latest"

		files
		{
			"src/Platforms/Linux/**.h",
			"src/Platforms/Linux/**.hpp",
			"src/Platforms/Linux/**.cpp",
		}

		libdirs
		{
			"../3rdParty/openal-soft/Release",
		}

		links
		{
			"uuid",
			"dl",
			"pthread",
			"openal:shared",
		}

		postbuildcommands
		{
			--("{COPY} %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/libGreenTea.so %{wks.location}/bin/" .. outputdir .. "/StandAlone"),
		}

    filter "configurations:Dist"
        defines "GT_DIST"
        runtime "Release"
        optimize "on"
        symbols "off"