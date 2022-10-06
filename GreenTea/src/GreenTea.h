#pragma once

#include <Engine/Core/Application.h>
#include <Engine/Core/Context.h>
#include <Engine/Core/FileDialog.h>
#include <Engine/Core/Math.h>
#include <Engine/Core/Random.h>

#include <Engine/Events/Input.h>

#include <Engine/GPU/FrameBuffer.h>
#include <Engine/GPU/Texture.h>//Maybe remove?

#include <Engine/ImGui/ImGuiLayer.h>
#include <Engine/ImGui/ImGuiWidgets.h>

#include <Engine/NativeScripting/ScriptableEntity.h>
#include <Engine/NativeScripting/System.h>

#include <Engine/Renderer/RenderCommand.h>

#include <Engine/Scene/Components.h>
#include <Engine/Scene/Entity.h>
#include <Engine/Scene/SceneSerializer.h>


#ifdef GREEN_TEA_ENTRY_POINT
	#include <Engine/Core/EntryPoint.h>
#endif