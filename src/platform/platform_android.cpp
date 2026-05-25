// Android platform layer
// Uses SDL2 + OpenGL ES for ImGui rendering

#include "imgui.h"
#include "ui/ui_main.h"
#include "app.h"
#include <android/log.h>

#define LOG_TAG "ResumeBuilder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Android entry point would be implemented here
// Requires SDL2 or native activity setup

namespace resume {
namespace android {

void init() {
    LOGI("ResumeBuilder Android init");
    ui_init();
}

void shutdown() {
    ui_shutdown();
    LOGI("ResumeBuilder Android shutdown");
}

void render() {
    ui_render();
}

} // namespace android
} // namespace resume
