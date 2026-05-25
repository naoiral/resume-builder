// Desktop platform layer (Windows, macOS, Linux)
// Uses ImGui's built-in backends for each platform

#include "imgui.h"
#include "ui/ui_main.h"
#include "app.h"
#include <string>
#include <iostream>

#ifdef _WIN32
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include <windows.h>
#include <gl/GL.h>
#include <shellapi.h>

// Forward declare
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static HWND g_hwnd = nullptr;
static HDC g_hdc = nullptr;
static HGLRC g_hglrc = nullptr;
static bool g_running = true;
static bool g_initialized = false;

namespace resume {
namespace platform {

// File dialog
std::string open_file_dialog() {
    OPENFILENAMEA ofn;
    char szFile[512] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "简历文件 (*.resume)\0*.resume\0所有文件 (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) return szFile;
    return "";
}

std::string save_file_dialog(const char* default_name) {
    OPENFILENAMEA ofn;
    char szFile[512] = "";
    if (default_name) strncpy(szFile, default_name, sizeof(szFile) - 1);
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "简历文件 (*.resume)\0*.resume\0PDF文件 (*.pdf)\0*.pdf\0所有文件 (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameA(&ofn)) return szFile;
    return "";
}

std::string save_pdf_dialog() {
    OPENFILENAMEA ofn;
    char szFile[512] = "";
    auto& app = resume::App::instance();
    if (!app.data.personal.name.empty()) {
        std::string suggested = app.data.personal.name + "-" + app.data.personal.objective + "-简历.pdf";
        strncpy(szFile, suggested.c_str(), sizeof(szFile) - 1);
    }
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "PDF文件 (*.pdf)\0*.pdf\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameA(&ofn)) return szFile;
    return "";
}

// Open file in explorer
void open_in_explorer(const std::string& path) {
    std::string cmd = "explorer /select,\"" + path + "\"";
    system(cmd.c_str());
}

} // namespace platform
} // namespace resume

// Window procedure
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_initialized && wParam != SIZE_MINIMIZED) {
            // Handle resize
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        char filename[MAX_PATH];
        if (DragQueryFileA(hDrop, 0, filename, MAX_PATH)) {
            resume::App::instance().open_file(filename);
        }
        DragFinish(hDrop);
        return 0;
    }
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// Create OpenGL context
static bool create_gl_context() {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0
    };

    g_hdc = GetDC(g_hwnd);
    int format = ChoosePixelFormat(g_hdc, &pfd);
    SetPixelFormat(g_hdc, format, &pfd);
    g_hglrc = wglCreateContext(g_hdc);
    wglMakeCurrent(g_hdc, g_hglrc);
    return true;
}

// Handle keyboard shortcuts
static void handle_shortcuts() {
    auto& io = ImGui::GetIO();
    auto& app = resume::App::instance();

    bool ctrl = io.KeyCtrl;
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
        app.new_resume();
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
        std::string path = resume::platform::open_file_dialog();
        if (!path.empty()) app.open_file(path);
    }
    if (ctrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S)) {
        std::string path = resume::platform::save_file_dialog("我的简历.resume");
        if (!path.empty()) app.save_file_as(path);
    } else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        if (app.current_file.empty()) {
            std::string path = resume::platform::save_file_dialog("我的简历.resume");
            if (!path.empty()) app.save_file_as(path);
        } else {
            app.save_file();
        }
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_E)) {
        std::string path = resume::platform::save_pdf_dialog();
        if (!path.empty()) app.export_pdf(path);
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        app.undo();
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
        app.redo();
    }
}

// Main entry point (Windows)
int main(int argc, char** argv) {
    return WinMain(GetModuleHandleA(nullptr), nullptr, GetCommandLineA(), SW_SHOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Create window
    WNDCLASSEXA wc = {
        sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr,
        "ResumeBuilder", nullptr
    };
    RegisterClassExA(&wc);

    g_hwnd = CreateWindowA("ResumeBuilder", "简历制作工具 v1.0.0",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800,
        nullptr, nullptr, wc.hInstance, nullptr);

    // Enable drag and drop
    DragAcceptFiles(g_hwnd, TRUE);

    // Create OpenGL context
    create_gl_context();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;  // Don't save imgui.ini

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load Chinese font (Microsoft YaHei)
    char win_dir[MAX_PATH];
    GetWindowsDirectoryA(win_dir, MAX_PATH);
    std::string font_path = std::string(win_dir) + "\\Fonts\\msyh.ttc";

    // Try Microsoft YaHei first, fallback to SimSun
    FILE* f = fopen(font_path.c_str(), "rb");
    if (!f) {
        font_path = std::string(win_dir) + "\\Fonts\\simsun.ttc";
        f = fopen(font_path.c_str(), "rb");
    }
    if (!f) {
        font_path = std::string(win_dir) + "\\Fonts\\msyhbd.ttc";
        f = fopen(font_path.c_str(), "rb");
    }

    if (f) {
        fclose(f);
        // Load font with Chinese glyph ranges
        ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromFileTTF(font_path.c_str(), 16.0f, &font_cfg,
            io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    } else {
        // Fallback: use default font with Chinese ranges
        io.Fonts->AddFontDefault();
    }

    // Initialize app
    resume::ui_init();

    // Auto-detect DPI
    float dpi_scale = 1.0f;
    HDC hdc = GetDC(g_hwnd);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(g_hwnd, hdc);
    if (dpi > 96) {
        dpi_scale = dpi / 96.0f;
    }
    io.FontGlobalScale = dpi_scale;

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    g_initialized = true;

    // Main loop
    MSG msg;
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                g_running = false;
        }
        if (!g_running) break;

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Handle shortcuts
        handle_shortcuts();

        // Render UI
        resume::ui_render();

        // Render ImGui
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.118f, 0.118f, 0.180f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(g_hdc);
    }

    // Cleanup
    resume::ui_shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(g_hglrc);
    ReleaseDC(g_hwnd, g_hdc);
    DestroyWindow(g_hwnd);
    UnregisterClassA("ResumeBuilder", wc.hInstance);

    return 0;
}

#else
// macOS/Linux entry point (stub for now)
int main(int argc, char** argv) {
    std::cout << "ResumeBuilder - macOS/Linux build" << std::endl;
    std::cout << "This platform requires GLFW/SDL2 backend integration." << std::endl;
    std::cout << "Please use the Windows build or configure platform-specific backend." << std::endl;
    return 0;
}

#endif
