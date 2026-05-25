#pragma once
#include "imgui.h"

namespace resume {
namespace theme {

// Catppuccin Mocha colors
constexpr ImU32 Base       = 0xFF1E1E2E;
constexpr ImU32 Mantle     = 0xFF181825;
constexpr ImU32 Crust      = 0xFF11111B;
constexpr ImU32 Surface0   = 0xFF313244;
constexpr ImU32 Surface1   = 0xFF45475A;
constexpr ImU32 Surface2   = 0xFF585B70;
constexpr ImU32 Overlay0   = 0xFF6C7086;
constexpr ImU32 Overlay1   = 0xFF7F849C;
constexpr ImU32 Subtext0   = 0xFFA6ADC8;
constexpr ImU32 Text       = 0xFFCDD6F4;
constexpr ImU32 Lavender   = 0xFFB4BEFE;
constexpr ImU32 Blue       = 0xFF89B4FA;
constexpr ImU32 Sky        = 0xFF89DCEB;
constexpr ImU32 Green      = 0xFFA6E3A1;
constexpr ImU32 Yellow     = 0xFFF9E2AF;
constexpr ImU32 Peach      = 0xFFFAB387;
constexpr ImU32 Maroon     = 0xFFEBA0AC;
constexpr ImU32 Red        = 0xFFF38BA8;
constexpr ImU32 Mauve      = 0xFFCBA6F7;
constexpr ImU32 Teal       = 0xFF94E2D5;

// ImVec4 versions for ImGui
constexpr ImVec4 Base_v    = ImVec4(0.118f, 0.118f, 0.180f, 1.0f);
constexpr ImVec4 Surface0_v= ImVec4(0.192f, 0.196f, 0.267f, 1.0f);
constexpr ImVec4 Surface1_v= ImVec4(0.271f, 0.278f, 0.353f, 1.0f);
constexpr ImVec4 Blue_v    = ImVec4(0.537f, 0.706f, 0.980f, 1.0f);
constexpr ImVec4 Green_v   = ImVec4(0.651f, 0.890f, 0.631f, 1.0f);
constexpr ImVec4 Text_v    = ImVec4(0.804f, 0.839f, 0.957f, 1.0f);

void apply_theme();
float get_scale_factor();

} // namespace theme
} // namespace resume
