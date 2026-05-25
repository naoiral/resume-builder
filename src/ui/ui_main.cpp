#include "ui_main.h"
#include "ui_editor.h"
#include "ui_preview.h"
#include "ui_theme.h"
#include "app.h"
#include "platform/platform.h"
#include "imgui.h"
#include <cstring>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace resume {

static bool show_about = false;
static bool show_save_success = false;
static std::string save_success_path;

static void render_menu_bar() {
    auto& app = App::instance();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("文件")) {
            if (ImGui::MenuItem("新建", "Ctrl+N")) {
                app.new_resume();
            }
            if (ImGui::MenuItem("打开...", "Ctrl+O")) {
                std::string path = platform::open_file_dialog();
                if (!path.empty()) app.open_file(path);
            }
            if (ImGui::MenuItem("保存", "Ctrl+S", false, !app.current_file.empty())) {
                if (app.save_file()) {
                    save_success_path = app.current_file;
                    show_save_success = true;
                }
            }
            if (ImGui::MenuItem("另存为...", "Ctrl+Shift+S")) {
                std::string path = platform::save_file_dialog("我的简历.resume");
                if (!path.empty()) {
                    if (app.save_file_as(path)) {
                        save_success_path = path;
                        show_save_success = true;
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("导出PDF...", "Ctrl+E")) {
                std::string path = platform::save_pdf_dialog();
                if (!path.empty()) {
                    if (app.export_pdf(path)) {
                        save_success_path = path;
                        show_save_success = true;
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("退出", "Alt+F4")) {
                // Handled by platform
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("编辑")) {
            if (ImGui::MenuItem("撤销", "Ctrl+Z", false, app.can_undo())) {
                app.undo();
            }
            if (ImGui::MenuItem("重做", "Ctrl+Y", false, app.can_redo())) {
                app.redo();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("模板")) {
            for (int i = 0; i < 4; i++) {
                if (ImGui::MenuItem(app.template_names[i], nullptr, app.selected_template == i)) {
                    app.selected_template = i;
                    app.push_undo();
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("帮助")) {
            if (ImGui::MenuItem("关于")) {
                show_about = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

static void render_welcome() {
    auto& app = App::instance();
    auto& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("欢迎使用简历制作工具", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
    ImGui::PushFont(nullptr);  // Will use default, larger font set in init
    ImGui::TextColored(ImVec4(0.537f, 0.706f, 0.980f, 1.0f), "简历制作工具");
    ImGui::PopFont();

    ImGui::TextColored(ImVec4(0.651f, 0.890f, 0.631f, 1.0f), "v1.0.0");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("完全离线的专业简历制作工具，支持多模板、实时预览、PDF导出。");
    ImGui::Spacing();

    ImGui::Text("功能特点：");
    ImGui::BulletText("4个专业简历模板");
    ImGui::BulletText("分模块可视化编辑");
    ImGui::BulletText("实时预览效果");
    ImGui::BulletText("一键导出PDF");
    ImGui::BulletText("自动保存，防止丢失");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float btn_width = 200;
    float total_width = btn_width * 2 + 20;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - total_width) * 0.5f);

    if (ImGui::Button("新建简历", ImVec2(btn_width, 45))) {
        app.new_resume();
    }
    ImGui::SameLine();
    if (ImGui::Button("打开简历", ImVec2(btn_width, 45))) {
        std::string path = platform::open_file_dialog();
        if (!path.empty()) app.open_file(path);
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.424f, 0.447f, 0.522f, 1.0f),
        "支持 .resume 文件格式，完全离线运行");

    ImGui::End();
}

static void render_about() {
    if (!show_about) return;
    ImGui::OpenPopup("关于");
    if (ImGui::BeginPopupModal("关于", &show_about, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.537f, 0.706f, 0.980f, 1.0f), "简历制作工具 v1.0.0");
        ImGui::Separator();
        ImGui::TextWrapped("完全离线的专业简历制作软件，支持Windows、macOS、Android、iOS全平台使用。");
        ImGui::Spacing();
        ImGui::Text("技术栈：");
        ImGui::BulletText("C++17 + ImGui + libharu");
        ImGui::BulletText("跨平台 CMake 构建");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.424f, 0.447f, 0.522f, 1.0f),
            "无使用限制，可自由分享传播");
        ImGui::Spacing();
        if (ImGui::Button("确定", ImVec2(120, 0))) {
            show_about = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void render_save_success() {
    if (!show_save_success) return;
    ImGui::OpenPopup("保存成功");
    if (ImGui::BeginPopupModal("保存成功", &show_save_success, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.651f, 0.890f, 0.631f, 1.0f), "保存成功！");
        ImGui::Separator();
        ImGui::TextWrapped("文件已保存到：");
        ImGui::TextColored(ImVec4(0.537f, 0.706f, 0.980f, 1.0f), "%s", save_success_path.c_str());
        ImGui::Spacing();
        if (ImGui::Button("打开文件夹", ImVec2(120, 0))) {
            platform::open_in_explorer(save_success_path);
            show_save_success = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("确定", ImVec2(120, 0))) {
            show_save_success = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void render_status_bar() {
    auto& app = App::instance();
    ImGui::Separator();

    float height = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("##statusbar", ImVec2(0, height), false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Status message
    if (app.status_timer > 0) {
        ImGui::TextColored(ImVec4(0.651f, 0.890f, 0.631f, 1.0f), "%s", app.status_message.c_str());
    }

    // Right side: file info
    ImGui::SameLine(ImGui::GetWindowWidth() - 300);
    if (!app.current_file.empty()) {
        ImGui::TextColored(ImVec4(0.424f, 0.447f, 0.522f, 1.0f), "%s", app.current_file.c_str());
    }
    if (app.modified) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.953f, 0.545f, 0.659f, 1.0f), "[未保存]");
    }

    ImGui::EndChild();
}

void ui_init() {
    auto& app = App::instance();
    app.init();
    theme::apply_theme();
}

void ui_shutdown() {
    App::instance().shutdown();
}

void ui_render() {
    auto& app = App::instance();

    // Update status timer
    if (app.status_timer > 0) {
        app.status_timer -= ImGui::GetIO().DeltaTime;
    }

    // Main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##main", nullptr, flags);

    render_menu_bar();

    if (app.current_view == AppView::Welcome) {
        render_welcome();
    } else {
        // Editor layout: left panel (45%) + right panel (55%)
        float width = ImGui::GetContentRegionAvail().x;
        float height = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - 10;

        // Left: Editor
        ImGui::BeginChild("##editor", ImVec2(width * 0.45f, height), true);
        ui_editor_render();
        ImGui::EndChild();

        ImGui::SameLine();

        // Right: Preview (larger)
        ImGui::BeginChild("##preview", ImVec2(width * 0.55f - 10, height), true);
        ui_preview_render();
        ImGui::EndChild();
    }

    render_status_bar();
    ImGui::End();

    render_about();
    render_save_success();
}

} // namespace resume
