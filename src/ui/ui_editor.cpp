#include "ui_editor.h"
#include "ui_theme.h"
#include "ui_preview.h"
#include "app.h"
#include "imgui.h"
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#endif

namespace resume {

// Helper: input text with label
static bool InputText(const char* label, std::string& buf, ImGuiInputTextFlags flags = 0) {
    char* data = const_cast<char*>(buf.c_str());
    int capacity = std::max((int)buf.size() + 256, 1024);
    buf.resize(capacity, '\0');
    bool changed = ImGui::InputText(label, const_cast<char*>(buf.data()), capacity,
        flags | ImGuiInputTextFlags_CallbackResize,
        [](ImGuiInputTextCallbackData* data) -> int {
            if (data->EventFlag & ImGuiInputTextFlags_CallbackResize) {
                std::string* str = (std::string*)data->UserData;
                str->resize(data->BufTextLen);
                data->Buf = const_cast<char*>(str->c_str());
            }
            return 0;
        }, &buf);
    buf.resize(strlen(buf.c_str()));
    return changed;
}

// Helper: multiline input
static bool InputTextMultiline(const char* label, std::string& buf, float height = 80) {
    char* data = const_cast<char*>(buf.c_str());
    int capacity = std::max((int)buf.size() + 256, 2048);
    buf.resize(capacity, '\0');
    bool changed = ImGui::InputTextMultiline(label, const_cast<char*>(buf.data()), capacity,
        ImVec2(-1, height),
        ImGuiInputTextFlags_CallbackResize,
        [](ImGuiInputTextCallbackData* data) -> int {
            if (data->EventFlag & ImGuiInputTextFlags_CallbackResize) {
                std::string* str = (std::string*)data->UserData;
                str->resize(data->BufTextLen);
                data->Buf = const_cast<char*>(str->c_str());
            }
            return 0;
        }, &buf);
    buf.resize(strlen(buf.c_str()));
    return changed;
}

// Section header with blue underline
static void SectionHeader(const char* title) {
    ImGui::PushStyleColor(ImGuiCol_Text, theme::Blue);
    ImGui::Text("%s", title);
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(min.x, max.y + 2), ImVec2(max.x, max.y + 2),
        theme::Blue, 2.0f);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

// Move up/down buttons for list items
static void ItemControls(int idx, int total, std::function<void(int, int)> move_fn,
                          std::function<void(int)> remove_fn) {
    ImGui::PushID(idx);
    if (ImGui::SmallButton("上移") && idx > 0) move_fn(idx, idx - 1);
    ImGui::SameLine();
    if (ImGui::SmallButton("下移") && idx < total - 1) move_fn(idx, idx + 1);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.953f, 0.545f, 0.659f, 0.6f));
    if (ImGui::SmallButton("删除")) remove_fn(idx);
    ImGui::PopStyleColor();
    ImGui::PopID();
}

// Personal info section
static void render_personal() {
    auto& app = App::instance();
    auto& p = app.data.personal;

    SectionHeader("个人信息");

    bool changed = false;
    ImGui::PushFont(nullptr);
    ImGui::SetWindowFontScale(1.2f);
    changed |= InputText("姓名##name", p.name);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    changed |= InputText("电话##phone", p.phone);
    changed |= InputText("邮箱##email", p.email);
    changed |= InputText("求职意向##objective", p.objective);

    // Photo selector - prominent
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("证件照：");
    ImGui::Spacing();

    if (p.photo_path.empty()) {
        // No photo yet - big upload button
        ImVec2 btn_size(200, 50);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 0.9f));
        if (ImGui::Button("+ 选择照片", btn_size)) {
#ifdef _WIN32
            OPENFILENAMEA ofn;
            char szFile[MAX_PATH] = {0};
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = nullptr;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileNameA(&ofn)) {
                app.push_undo();
                p.photo_path = szFile;
                changed = true;
            }
#endif
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "支持 PNG / JPG 格式");
    } else {
        // Has photo - show preview with actions
        unsigned int tex = load_texture(p.photo_path);
        if (tex) {
            // Photo preview with border
            ImVec2 img_size(120, 160);
            ImGui::BeginChild("##photo_preview", ImVec2(img_size.x + 16, img_size.y + 50), true);
            ImGui::Image((ImTextureID)(uintptr_t)tex, img_size);
            ImGui::Spacing();
            if (ImGui::Button("更换照片", ImVec2(img_size.x * 0.48f, 0))) {
#ifdef _WIN32
                OPENFILENAMEA ofn;
                char szFile[MAX_PATH] = {0};
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = nullptr;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileNameA(&ofn)) {
                    app.push_undo();
                    p.photo_path = szFile;
                    changed = true;
                }
#endif
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.3f, 0.3f, 0.7f));
            if (ImGui::Button("移除", ImVec2(img_size.x * 0.48f, 0))) {
                app.push_undo();
                p.photo_path.clear();
                changed = true;
            }
            ImGui::PopStyleColor();
            ImGui::EndChild();
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    changed |= InputTextMultiline("个人简介##summary", p.summary, 60);

    if (changed) app.push_undo();
}

// Education section
static void render_education() {
    auto& app = App::instance();
    auto& edu = app.data.education;

    SectionHeader("教育经历");

    if (ImGui::SmallButton("+ 添加教育经历")) {
        app.push_undo();
        edu.push_back(Education());
    }

    for (int i = 0; i < (int)edu.size(); i++) {
        ImGui::PushID(i);
        ImGui::Separator();
        ImGui::Spacing();

        auto move_fn = [&](int from, int to) {
            app.push_undo();
            std::swap(edu[from], edu[to]);
        };
        auto remove_fn = [&](int idx) {
            app.push_undo();
            edu.erase(edu.begin() + idx);
        };

        ItemControls(i, (int)edu.size(), move_fn, remove_fn);

        bool changed = false;
        changed |= InputText("学校##school", edu[i].school);
        changed |= InputText("专业##major", edu[i].major);
        changed |= InputText("学历##degree", edu[i].degree);

        ImGui::Columns(2, nullptr, false);
        changed |= InputText("起始时间##start", edu[i].start_date);
        ImGui::NextColumn();
        changed |= InputText("结束时间##end", edu[i].end_date);
        ImGui::Columns(1);

        changed |= InputText("GPA##gpa", edu[i].gpa);
        changed |= InputText("主修课程##courses", edu[i].courses);

        if (changed) app.push_undo();
        ImGui::PopID();
    }
}

// Work experience section
static void render_work() {
    auto& app = App::instance();
    auto& work = app.data.work;

    SectionHeader("工作/实习经历");

    if (ImGui::SmallButton("+ 添加工作经历")) {
        app.push_undo();
        work.push_back(WorkExperience());
    }

    for (int i = 0; i < (int)work.size(); i++) {
        ImGui::PushID(1000 + i);
        ImGui::Separator();
        ImGui::Spacing();

        auto move_fn = [&](int from, int to) {
            app.push_undo();
            std::swap(work[from], work[to]);
        };
        auto remove_fn = [&](int idx) {
            app.push_undo();
            work.erase(work.begin() + idx);
        };

        ItemControls(i, (int)work.size(), move_fn, remove_fn);

        bool changed = false;
        changed |= InputText("公司名称##company", work[i].company);
        changed |= InputText("职位##title", work[i].title);

        ImGui::Columns(2, nullptr, false);
        changed |= InputText("起始时间##start", work[i].start_date);
        ImGui::NextColumn();
        changed |= InputText("结束时间##end", work[i].end_date);
        ImGui::Columns(1);

        // Duties
        ImGui::Text("工作内容：");
        for (int d = 0; d < (int)work[i].duties.size(); d++) {
            ImGui::PushID(d);
            char label[32];
            snprintf(label, sizeof(label), "##duty%d", d);
            changed |= InputTextMultiline(label, work[i].duties[d], 40);
            ImGui::SameLine();
            if (ImGui::SmallButton("-")) {
                app.push_undo();
                work[i].duties.erase(work[i].duties.begin() + d);
                d--;
            }
            ImGui::PopID();
        }
        if (ImGui::SmallButton("+ 添加工作内容")) {
            app.push_undo();
            work[i].duties.push_back("");
        }

        if (changed) app.push_undo();
        ImGui::PopID();
    }
}

// Project section
static void render_projects() {
    auto& app = App::instance();
    auto& projects = app.data.projects;

    SectionHeader("项目经历");

    if (ImGui::SmallButton("+ 添加项目")) {
        app.push_undo();
        projects.push_back(Project());
    }

    for (int i = 0; i < (int)projects.size(); i++) {
        ImGui::PushID(2000 + i);
        ImGui::Separator();
        ImGui::Spacing();

        auto move_fn = [&](int from, int to) {
            app.push_undo();
            std::swap(projects[from], projects[to]);
        };
        auto remove_fn = [&](int idx) {
            app.push_undo();
            projects.erase(projects.begin() + idx);
        };

        ItemControls(i, (int)projects.size(), move_fn, remove_fn);

        bool changed = false;
        changed |= InputText("项目名称##name", projects[i].name);

        ImGui::Columns(2, nullptr, false);
        changed |= InputText("起始时间##start", projects[i].start_date);
        ImGui::NextColumn();
        changed |= InputText("结束时间##end", projects[i].end_date);
        ImGui::Columns(1);

        changed |= InputTextMultiline("项目描述##desc", projects[i].description, 50);
        changed |= InputText("技术栈##tech", projects[i].tech_stack);

        // Duties
        ImGui::Text("个人职责与成果：");
        for (int d = 0; d < (int)projects[i].duties.size(); d++) {
            ImGui::PushID(d);
            char label[32];
            snprintf(label, sizeof(label), "##duty%d", d);
            changed |= InputTextMultiline(label, projects[i].duties[d], 40);
            ImGui::SameLine();
            if (ImGui::SmallButton("-")) {
                app.push_undo();
                projects[i].duties.erase(projects[i].duties.begin() + d);
                d--;
            }
            ImGui::PopID();
        }
        if (ImGui::SmallButton("+ 添加职责")) {
            app.push_undo();
            projects[i].duties.push_back("");
        }

        if (changed) app.push_undo();
        ImGui::PopID();
    }
}

// Skills section
static void render_skills() {
    auto& app = App::instance();
    auto& skills = app.data.skills;

    SectionHeader("技能特长");

    if (ImGui::SmallButton("+ 添加技能分类")) {
        app.push_undo();
        skills.push_back(SkillCategory());
    }

    for (int i = 0; i < (int)skills.size(); i++) {
        ImGui::PushID(3000 + i);
        ImGui::Separator();
        ImGui::Spacing();

        auto move_fn = [&](int from, int to) {
            app.push_undo();
            std::swap(skills[from], skills[to]);
        };
        auto remove_fn = [&](int idx) {
            app.push_undo();
            skills.erase(skills.begin() + idx);
        };

        ItemControls(i, (int)skills.size(), move_fn, remove_fn);

        bool changed = false;
        changed |= InputText("分类名称##cat", skills[i].category);

        // Skill items
        for (int s = 0; s < (int)skills[i].items.size(); s++) {
            ImGui::PushID(s);
            changed |= InputText("##skill_name", skills[i].items[s].name);
            ImGui::SameLine();

            // Level selector
            const char* levels[] = {"了解", "熟悉", "掌握", "熟练"};
            int level = static_cast<int>(skills[i].items[s].level);
            char combo_label[32];
            snprintf(combo_label, sizeof(combo_label), "##level%d", s);
            if (ImGui::BeginCombo(combo_label, levels[level])) {
                for (int l = 0; l < 4; l++) {
                    if (ImGui::Selectable(levels[l], level == l)) {
                        skills[i].items[s].level = static_cast<SkillLevel>(l);
                        changed = true;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            if (ImGui::SmallButton("-")) {
                app.push_undo();
                skills[i].items.erase(skills[i].items.begin() + s);
                s--;
            }
            ImGui::PopID();
        }

        if (ImGui::SmallButton("+ 添加技能")) {
            app.push_undo();
            skills[i].items.push_back({"", SkillLevel::Familiar});
        }

        if (changed) app.push_undo();
        ImGui::PopID();
    }
}

// Certificates section
static void render_certificates() {
    auto& app = App::instance();
    auto& certs = app.data.certificates;

    SectionHeader("证书奖项");

    if (ImGui::SmallButton("+ 添加证书")) {
        app.push_undo();
        certs.push_back(Certificate());
    }

    for (int i = 0; i < (int)certs.size(); i++) {
        ImGui::PushID(4000 + i);
        ImGui::Separator();
        ImGui::Spacing();

        auto move_fn = [&](int from, int to) {
            app.push_undo();
            std::swap(certs[from], certs[to]);
        };
        auto remove_fn = [&](int idx) {
            app.push_undo();
            certs.erase(certs.begin() + idx);
        };

        ItemControls(i, (int)certs.size(), move_fn, remove_fn);

        bool changed = false;
        ImGui::Columns(2, nullptr, false);
        changed |= InputText("证书名称##name", certs[i].name);
        ImGui::NextColumn();
        changed |= InputText("获得时间##date", certs[i].date);
        ImGui::Columns(1);

        if (changed) app.push_undo();
        ImGui::PopID();
    }
}

void ui_editor_render() {
    auto& app = App::instance();

    // Template selector - first line
    ImGui::Text("模板：");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo("##template", app.template_names[app.selected_template])) {
        for (int i = 0; i < 4; i++) {
            if (ImGui::Selectable(app.template_names[i], app.selected_template == i)) {
                app.selected_template = i;
                app.push_undo();
            }
        }
        ImGui::EndCombo();
    }

    // Text color - second line
    ImGui::Text("文字：");
    ImGui::SameLine();
    ImGui::ColorEdit4("##text_color", app.text_color,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    ImGui::SameLine();
    if (ImGui::SmallButton("黑##t")) {
        app.text_color[0] = 0.0f; app.text_color[1] = 0.0f; app.text_color[2] = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("蓝##t")) {
        app.text_color[0] = 0.1f; app.text_color[1] = 0.1f; app.text_color[2] = 0.5f;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("灰##t")) {
        app.text_color[0] = 0.25f; app.text_color[1] = 0.25f; app.text_color[2] = 0.25f;
    }

    // Border/line color - third line
    ImGui::Text("边框：");
    ImGui::SameLine();
    ImGui::ColorEdit4("##border_color", app.border_color,
        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    ImGui::SameLine();
    if (ImGui::SmallButton("黑##b")) {
        app.border_color[0] = 0.0f; app.border_color[1] = 0.0f; app.border_color[2] = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("蓝##b")) {
        app.border_color[0] = 0.2f; app.border_color[1] = 0.4f; app.border_color[2] = 0.8f;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("灰##b")) {
        app.border_color[0] = 0.5f; app.border_color[1] = 0.5f; app.border_color[2] = 0.5f;
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Tab bar for modules
    if (ImGui::BeginTabBar("##modules")) {
        if (ImGui::BeginTabItem("个人信息")) {
            render_personal();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("教育经历")) {
            render_education();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("工作经历")) {
            render_work();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("项目经历")) {
            render_projects();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("技能特长")) {
            render_skills();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("证书奖项")) {
            render_certificates();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

} // namespace resume
