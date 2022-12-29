#ifndef GT_DIST
#include "GLFWImGuiLayer.h"
#include <Engine/Core/Context.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <IconsForkAwesome.h>

static void SetDarkThemeColors(void);

namespace gte::GLFW {
	
	void GLFWImGuiLayer::Init(void* window, GPU::GraphicsContext* context)
	{
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        mContext = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        static constexpr ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
        io.Fonts->AddFontFromFileTTF("../Assets/Fonts/Icons/ForkAwesome/" FONT_ICON_FILE_NAME_FK, 13.0f, NULL, icon_ranges);
        io.Fonts->AddFontFromFileTTF("../Assets/Fonts/Roboto/Roboto-Bold.ttf", 18);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("../Assets/Fonts/Roboto/Roboto-Regular.ttf", 18);
        io.Fonts->AddFontFromFileTTF("../Assets/Fonts/Icons/ForkAwesome/" FONT_ICON_FILE_NAME_FK, 18.0f, NULL, icon_ranges);
        io.Fonts->AddFontFromFileTTF("../Assets/Fonts/Roboto/Roboto-Regular.ttf", 32);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 6.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        style.ChildBorderSize = 0.0f;
        style.WindowBorderSize = 0.0f;
        style.PopupBorderSize = 0.0f;

        SetDarkThemeColors();

        // Setup Platform/Renderer backends
        glfwMakeContextCurrent((GLFWwindow*)window);
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window, true);
        ImGui_ImplOpenGL3_Init("#version 410 core");
	}

	void GLFWImGuiLayer::Begin(void)
	{
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
	}

	void GLFWImGuiLayer::End(void)
	{
        Window* window = internal::GetContext()->GlobalWindow;
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
	}

	void GLFWImGuiLayer::Shutdown(void)
	{
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
	}

	[[nodiscard]] ImGuiContext* GLFWImGuiLayer::GetContext(void) noexcept { return mContext; }
}

namespace gte::gui { [[nodiscard]] ImGuiLayer* ImGuiLayer::Create(void) noexcept { return new GLFW::GLFWImGuiLayer(); } }

void SetDarkThemeColors(void)
{
    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

    // Headers
    colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}
#endif