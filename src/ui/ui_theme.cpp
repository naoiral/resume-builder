#include "ui_theme.h"

namespace resume {
namespace theme {

void apply_theme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]        = Base_v;
    colors[ImGuiCol_ChildBg]         = ImVec4(0.157f, 0.161f, 0.235f, 1.0f);
    colors[ImGuiCol_PopupBg]         = ImVec4(0.157f, 0.161f, 0.235f, 0.98f);
    colors[ImGuiCol_Border]          = ImVec4(0.271f, 0.278f, 0.353f, 0.5f);
    colors[ImGuiCol_FrameBg]         = ImVec4(0.192f, 0.196f, 0.267f, 1.0f);
    colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.271f, 0.278f, 0.353f, 1.0f);
    colors[ImGuiCol_FrameBgActive]   = ImVec4(0.337f, 0.357f, 0.439f, 1.0f);
    colors[ImGuiCol_TitleBg]         = ImVec4(0.094f, 0.094f, 0.145f, 1.0f);
    colors[ImGuiCol_TitleBgActive]   = ImVec4(0.118f, 0.118f, 0.180f, 1.0f);
    colors[ImGuiCol_MenuBarBg]       = ImVec4(0.118f, 0.118f, 0.180f, 1.0f);
    colors[ImGuiCol_ScrollbarBg]     = ImVec4(0.094f, 0.094f, 0.145f, 0.5f);
    colors[ImGuiCol_ScrollbarGrab]   = ImVec4(0.271f, 0.278f, 0.353f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.337f, 0.357f, 0.439f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.424f, 0.447f, 0.522f, 1.0f);
    colors[ImGuiCol_CheckMark]       = Blue_v;
    colors[ImGuiCol_SliderGrab]      = Blue_v;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.651f, 0.890f, 0.631f, 1.0f);
    colors[ImGuiCol_Button]          = ImVec4(0.337f, 0.357f, 0.439f, 1.0f);
    colors[ImGuiCol_ButtonHovered]   = ImVec4(0.424f, 0.447f, 0.522f, 1.0f);
    colors[ImGuiCol_ButtonActive]    = ImVec4(0.537f, 0.706f, 0.980f, 0.8f);
    colors[ImGuiCol_Header]          = ImVec4(0.337f, 0.357f, 0.439f, 1.0f);
    colors[ImGuiCol_HeaderHovered]   = ImVec4(0.424f, 0.447f, 0.522f, 1.0f);
    colors[ImGuiCol_HeaderActive]    = ImVec4(0.537f, 0.706f, 0.980f, 0.8f);
    colors[ImGuiCol_Separator]       = ImVec4(0.271f, 0.278f, 0.353f, 0.5f);
    colors[ImGuiCol_Tab]             = ImVec4(0.192f, 0.196f, 0.267f, 1.0f);
    colors[ImGuiCol_TabHovered]      = ImVec4(0.337f, 0.357f, 0.439f, 1.0f);
    colors[ImGuiCol_TabActive]       = ImVec4(0.271f, 0.278f, 0.353f, 1.0f);
    colors[ImGuiCol_Text]            = Text_v;
    colors[ImGuiCol_TextDisabled]    = ImVec4(0.424f, 0.447f, 0.522f, 1.0f);

    // Style
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.ScrollbarSize     = 12;
    style.GrabMinSize       = 8;
    style.WindowRounding    = 8;
    style.FrameRounding     = 6;
    style.PopupRounding     = 6;
    style.ScrollbarRounding = 6;
    style.GrabRounding      = 4;
    style.TabRounding       = 6;
    style.WindowBorderSize  = 1;
    style.FrameBorderSize   = 0;
    style.PopupBorderSize   = 1;
}

float get_scale_factor() {
#ifdef PLATFORM_MOBILE
    return 1.5f;
#else
    // Auto-detect DPI on desktop
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x > 0) {
        // Simple heuristic: scale up on high-DPI displays
        float scale = io.DisplayFramebufferScale.x;
        return scale > 1.0f ? scale : 1.0f;
    }
    return 1.0f;
#endif
}

} // namespace theme
} // namespace resume
