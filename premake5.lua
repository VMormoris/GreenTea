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
IncludeDirs["entt"]="%{wks.location}/3rdParty/entt/single_include/entt"
IncludeDirs["yaml"]="%{wks.location}/3rdParty/yaml-cpp/include"
IncludeDirs["GLFW"]="%{wks.location}/3rdParty/glfw/include"
IncludeDirs["Glad"]="%{wks.location}/3rdParty/glad/include"
IncludeDirs["glm"]="%{wks.location}/3rdParty/glm/glm"
IncludeDirs["stb"]="%{wks.location}/3rdParty/stb"
IncludeDirs["imgui"]="%{wks.location}/3rdParty/imgui"
IncludeDirs["IconHeader"]="%{wks.location}/3rdParty/IconFontCppHeaders"
IncludeDirs["box2d"]="%{wks.location}/3rdParty/box2d/include"
IncludeDirs["ImGuizmo"]="%{wks.location}/3rdParty/ImGuizmo"
IncludeDirs["libsndfile"]="%{wks.location}/3rdParty/libsndfile/include"
IncludeDirs["openal"]="%{wks.location}/3rdParty/openal-soft/include"
IncludeDirs["msdfgen"]="%{wks.location}/3rdParty/msdf-atlas-gen/msdfgen"
IncludeDirs["msdfatlasgen"]="%{wks.location}/3rdParty/msdf-atlas-gen"

group "Dependecies"
	include "3rdParty/imgui"
	include "3rdParty/box2d"
	include "3rdParty/glad"
	include "3rdParty/yaml-cpp"
	include "3rdParty/glfw"
	include "3rdParty/libsndfile"
	include "3rdParty/msdf-atlas-gen/msdfgen"
	include "3rdParty/msdf-atlas-gen"
group ""

group "Core"
	include "GreenTea"
	include "CupOfTea"
group ""