// iOS platform layer
// Uses UIKit + Metal for ImGui rendering

#include "imgui.h"
#include "ui/ui_main.h"
#include "app.h"

// iOS entry point would be implemented in Swift/ObjC
// This file provides the C++ bridge

namespace resume {
namespace ios {

void init() {
    ui_init();
}

void shutdown() {
    ui_shutdown();
}

void render() {
    ui_render();
}

} // namespace ios
} // namespace resume
