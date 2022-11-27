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

	removefiles
	{
		"src/Engine/ImGui/**.h",
		"src/Engine/ImGui/**.hpp",
		"src/Engine/ImGui/**.cpp",
	}

    includedirs
	{
		"src",
		"%{IncludeDirs.entt}",
        "%{IncludeDirs.yaml}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.stb}",
		"%{IncludeDirs.box2d}",
	}

    links
    {
        "Box2D",
        "yaml-cpp",
    }

    defines
	{
		"ENGINE_DLL",
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"AL_LIBTYPE_STATIC",
	}
    
    --postbuildcommands
    --{
    --    ("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.dll %{wks.location}bin\\" .. outputdir .. "\\StandAlone"),
	--	("{COPY} %{wks.location}bin\\" .. outputdir .. "\\%{prj.name}\\**.lib %{wks.location}bin\\" .. outputdir .. "\\StandAlone"),
    --}

    filter "system:windows"
		systemversion "latest"

		disablewarnings {4251, 4275}
		
		files
		{
			"src/Platforms/Windows/**.h",
			"src/Platforms/Windows/**.cpp",
		}

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

	filter "system:linux"
		pic "On"
		systemversion "latest"

		files
		{
			"src/Platforms/Linux/**.h",
			"src/Platforms/Linux/**.hpp",
			"src/Platforms/Linux/**.cpp",
		}

		links
		{
			"uuid",
			"dl",
			"pthread",
		}

	--buildoptions { "-mwasm64" }

	filter { "system:linux", "configurations:Dist" }
		
		libdirs
		{
			"../3rdParty/openal-soft/Release",
		}

		links
		{
			"openal:shared",
		}

    filter "configurations:Dist"
        defines "GT_DIST"
        runtime "Release"
        optimize "on"
        symbols "off"

		includedirs
		{
			"%{IncludeDirs.Glad}",
			"%{IncludeDirs.GLFW}",
			"%{IncludeDirs.openal}",
		}

		links
		{
			"GLFW",
			"Glad",
		}

	filter "configurations:Web"
		defines { "GT_DIST", "GT_WEB" }
        runtime "Release"
        optimize "on"
        symbols "off"