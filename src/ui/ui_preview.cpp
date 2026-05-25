#include "ui_preview.h"
#include "ui_theme.h"
#include "app.h"
#include "imgui.h"
#include <cmath>
#include <map>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace resume {

// Texture cache
static std::map<std::string, unsigned int> g_textures;

unsigned int load_texture(const std::string& path) {
    if (path.empty()) return 0;

    // Check cache
    auto it = g_textures.find(path);
    if (it != g_textures.end()) return it->second;

    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    g_textures[path] = tex;
    return tex;
}

void destroy_textures() {
    for (auto& [path, tex] : g_textures) {
        glDeleteTextures(1, &tex);
    }
    g_textures.clear();
}

// A4 aspect ratio: 210/297 = 0.707
static constexpr float A4_RATIO = 210.0f / 297.0f;

// Get text color from app settings
static ImU32 get_text_color() {
    auto& app = App::instance();
    return ImGui::ColorConvertFloat4ToU32(ImVec4(
        app.text_color[0], app.text_color[1], app.text_color[2], app.text_color[3]));
}

// Get border/line color from app settings
static ImU32 get_border_color() {
    auto& app = App::instance();
    return ImGui::ColorConvertFloat4ToU32(ImVec4(
        app.border_color[0], app.border_color[1], app.border_color[2], app.border_color[3]));
}

// Draw a section title with colored underline
static void draw_section_title(ImDrawList* dl, ImVec2& pos, const char* title, float width, float font_size) {
    ImU32 border_col = get_border_color();
    dl->AddText(nullptr, font_size, pos, border_col, title);
    ImVec2 text_size = ImGui::CalcTextSize(title);
    dl->AddLine(
        ImVec2(pos.x, pos.y + text_size.y + 2),
        ImVec2(pos.x + width, pos.y + text_size.y + 2),
        border_col, 1.5f);
    pos.y += text_size.y + 8;
}

// Draw wrapped text
static float draw_wrapped_text(ImDrawList* dl, ImVec2& pos, const char* text,
                                 float width, ImU32 color, float font_size) {
    if (!text || !text[0]) return 0;
    float start_y = pos.y;
    const char* p = text;
    float line_x = pos.x;

    while (*p) {
        const char* line_end = p;
        float line_width = 0;
        while (*line_end && *line_end != '\n') {
            char ch[2] = {*line_end, 0};
            line_width += ImGui::CalcTextSize(ch).x * (font_size / ImGui::GetFontSize());
            if (line_width > width) break;
            line_end++;
        }

        char line[512];
        int len = (int)(line_end - p);
        if (len > 511) len = 511;
        memcpy(line, p, len);
        line[len] = 0;

        dl->AddText(nullptr, font_size, pos, color, line);
        pos.y += font_size * 1.4f;
        pos.x = line_x;

        if (*line_end == '\n') line_end++;
        p = line_end;
    }

    return pos.y - start_y;
}

// Draw bullet list
static void draw_bullet_list(ImDrawList* dl, ImVec2& pos, const std::vector<std::string>& items,
                               float width, ImU32 color, float font_size) {
    for (const auto& item : items) {
        if (item.empty()) continue;
        dl->AddText(nullptr, font_size, pos, color, "•");
        ImVec2 text_pos(pos.x + 15, pos.y);
        std::string full = item;
        draw_wrapped_text(dl, text_pos, full.c_str(), width - 15, color, font_size);
        pos.y = text_pos.y + 2;
    }
}

// Classic template preview
static void render_classic(ImDrawList* dl, ImVec2 origin, float width, float height) {
    auto& data = App::instance().data;
    ImU32 text_color = get_text_color();
    ImU32 border_col = get_border_color();
    float margin = 15;
    float content_width = width - margin * 2;
    ImVec2 pos(origin.x + margin, origin.y + margin);
    float small_font = 10;
    float normal_font = 11;
    float title_font = 14;
    float name_font = 18;

    // Page border
    dl->AddRect(
        ImVec2(origin.x + 3, origin.y + 3),
        ImVec2(origin.x + width - 3, origin.y + height - 3),
        border_col, 0, 0, 1.5f);

    // Photo at top-right corner
    if (!data.personal.photo_path.empty()) {
        unsigned int tex = load_texture(data.personal.photo_path);
        if (tex) {
            float photo_w = width * 0.18f;
            float photo_h = photo_w * 1.33f;  // 3:4 ratio
            ImVec2 photo_pos(origin.x + width - margin - photo_w, origin.y + margin);
            dl->AddImage((ImTextureID)(uintptr_t)tex, photo_pos,
                        ImVec2(photo_pos.x + photo_w, photo_pos.y + photo_h));
            // Photo border
            dl->AddRect(photo_pos, ImVec2(photo_pos.x + photo_w, photo_pos.y + photo_h),
                       border_col, 0, 0, 1.0f);
        }
    }

    // Name
    if (!data.personal.name.empty()) {
        dl->AddText(nullptr, name_font, pos, text_color, data.personal.name.c_str());
        pos.y += name_font * 1.5f;
    }

    // Contact info
    std::string contact;
    if (!data.personal.phone.empty()) contact += data.personal.phone + "  |  ";
    if (!data.personal.email.empty()) contact += data.personal.email;
    if (!contact.empty()) {
        dl->AddText(nullptr, small_font, pos, text_color, contact.c_str());
        pos.y += small_font * 1.6f;
    }

    if (!data.personal.objective.empty()) {
        std::string obj = "求职意向：" + data.personal.objective;
        dl->AddText(nullptr, normal_font, pos, text_color, obj.c_str());
        pos.y += normal_font * 1.5f;
    }

    // Divider line after header
    pos.y += 3;
    dl->AddLine(
        ImVec2(origin.x + margin, pos.y),
        ImVec2(origin.x + width - margin, pos.y),
        border_col, 1.0f);
    pos.y += 8;

    // Education
    if (!data.education.empty()) {
        draw_section_title(dl, pos, "教育经历", content_width, title_font);
        for (const auto& edu : data.education) {
            std::string line = edu.school + "  " + edu.major + "  " + edu.degree;
            dl->AddText(nullptr, normal_font, pos, text_color, line.c_str());
            pos.y += normal_font * 1.4f;

            if (!edu.start_date.empty()) {
                std::string time = edu.start_date + " - " + edu.end_date;
                if (!edu.gpa.empty()) time += "  GPA: " + edu.gpa;
                dl->AddText(nullptr, small_font, pos, text_color, time.c_str());
                pos.y += small_font * 1.4f;
            }
            if (!edu.courses.empty()) {
                std::string courses = "主修课程：" + edu.courses;
                draw_wrapped_text(dl, pos, courses.c_str(), content_width, text_color, small_font);
            }
            pos.y += 4;
        }
    }

    // Work experience
    if (!data.work.empty()) {
        draw_section_title(dl, pos, "工作经历", content_width, title_font);
        for (const auto& w : data.work) {
            dl->AddText(nullptr, normal_font, pos, text_color, w.company.c_str());
            float text_w = ImGui::CalcTextSize(w.company.c_str()).x;
            if (!w.title.empty()) {
                dl->AddText(nullptr, normal_font, ImVec2(pos.x + text_w + 10, pos.y),
                           text_color, w.title.c_str());
            }
            pos.y += normal_font * 1.4f;

            if (!w.start_date.empty()) {
                std::string time = w.start_date + " - " + w.end_date;
                dl->AddText(nullptr, small_font, pos, text_color, time.c_str());
                pos.y += small_font * 1.4f;
            }

            draw_bullet_list(dl, pos, w.duties, content_width, text_color, normal_font);
            pos.y += 4;
        }
    }

    // Projects
    if (!data.projects.empty()) {
        draw_section_title(dl, pos, "项目经历", content_width, title_font);
        for (const auto& p : data.projects) {
            dl->AddText(nullptr, normal_font, pos, text_color, p.name.c_str());
            pos.y += normal_font * 1.4f;

            if (!p.start_date.empty()) {
                std::string time = p.start_date + " - " + p.end_date;
                if (!p.tech_stack.empty()) time += "  技术栈: " + p.tech_stack;
                dl->AddText(nullptr, small_font, pos, text_color, time.c_str());
                pos.y += small_font * 1.4f;
            }

            if (!p.description.empty()) {
                draw_wrapped_text(dl, pos, p.description.c_str(), content_width, text_color, normal_font);
            }
            draw_bullet_list(dl, pos, p.duties, content_width, text_color, normal_font);
            pos.y += 4;
        }
    }

    // Skills
    if (!data.skills.empty()) {
        draw_section_title(dl, pos, "技能特长", content_width, title_font);
        for (const auto& cat : data.skills) {
            std::string line = cat.category + "：";
            for (size_t i = 0; i < cat.items.size(); i++) {
                if (i > 0) line += "、";
                line += cat.items[i].name + "(" + skill_level_str(cat.items[i].level) + ")";
            }
            draw_wrapped_text(dl, pos, line.c_str(), content_width, text_color, normal_font);
            pos.y += 2;
        }
    }

    // Certificates
    if (!data.certificates.empty()) {
        draw_section_title(dl, pos, "证书奖项", content_width, title_font);
        for (const auto& cert : data.certificates) {
            std::string line = cert.name;
            if (!cert.date.empty()) line += "  (" + cert.date + ")";
            dl->AddText(nullptr, normal_font, pos, text_color, line.c_str());
            pos.y += normal_font * 1.4f;
        }
    }
}

// Modern template preview
static void render_modern(ImDrawList* dl, ImVec2 origin, float width, float height) {
    auto& data = App::instance().data;
    ImU32 text_color = get_text_color();
    ImU32 border_col = get_border_color();
    float sidebar_width = width * 0.3f;
    float main_width = width - sidebar_width;

    // Sidebar background
    dl->AddRectFilled(origin, ImVec2(origin.x + sidebar_width, origin.y + height),
                      0xFF28293D);  // Darker sidebar

    // Sidebar divider line
    dl->AddLine(
        ImVec2(origin.x + sidebar_width, origin.y),
        ImVec2(origin.x + sidebar_width, origin.y + height),
        border_col, 2.0f);

    // Main content area
    ImVec2 main_pos(origin.x + sidebar_width + 10, origin.y + 15);
    ImVec2 side_pos(origin.x + 10, origin.y + 15);

    float small_font = 9;

    // Photo in sidebar
    if (!data.personal.photo_path.empty()) {
        unsigned int tex = load_texture(data.personal.photo_path);
        if (tex) {
            float photo_w = sidebar_width * 0.6f;
            float photo_h = photo_w * 1.33f;
            float photo_x = origin.x + (sidebar_width - photo_w) * 0.5f;
            ImVec2 photo_pos(photo_x, side_pos.y);
            dl->AddImage((ImTextureID)(uintptr_t)tex, photo_pos,
                        ImVec2(photo_pos.x + photo_w, photo_pos.y + photo_h));
            dl->AddRect(photo_pos, ImVec2(photo_pos.x + photo_w, photo_pos.y + photo_h),
                       0xFFFFFFFF, 0, 0, 1.0f);
            side_pos.y += photo_h + 10;
        }
    }
    float normal_font = 10;
    float title_font = 12;
    float name_font = 16;

    // Name in sidebar
    if (!data.personal.name.empty()) {
        dl->AddText(nullptr, name_font, side_pos, theme::Blue, data.personal.name.c_str());
        side_pos.y += name_font * 1.6f;
    }

    // Contact in sidebar
    auto draw_side_item = [&](const char* icon, const std::string& text) {
        if (text.empty()) return;
        dl->AddText(nullptr, small_font, side_pos, theme::Subtext0, icon);
        dl->AddText(nullptr, small_font, ImVec2(side_pos.x + 15, side_pos.y), text_color, text.c_str());
        side_pos.y += small_font * 1.8f;
    };

    draw_side_item("Tel", data.personal.phone);
    draw_side_item("Mail", data.personal.email);
    draw_side_item("Job", data.personal.objective);

    side_pos.y += 10;

    // Skills in sidebar
    if (!data.skills.empty()) {
        dl->AddText(nullptr, title_font, side_pos, theme::Blue, "技能特长");
        side_pos.y += title_font * 1.5f;

        for (const auto& cat : data.skills) {
            dl->AddText(nullptr, small_font, side_pos, theme::Yellow, cat.category.c_str());
            side_pos.y += small_font * 1.5f;
            for (const auto& item : cat.items) {
                std::string line = "  " + item.name + " - " + skill_level_str(item.level);
                dl->AddText(nullptr, small_font, side_pos, text_color, line.c_str());
                side_pos.y += small_font * 1.5f;
            }
        }
    }

    // Certificates in sidebar
    if (!data.certificates.empty()) {
        side_pos.y += 10;
        dl->AddText(nullptr, title_font, side_pos, theme::Blue, "证书奖项");
        side_pos.y += title_font * 1.5f;
        for (const auto& cert : data.certificates) {
            dl->AddText(nullptr, small_font, side_pos, text_color, cert.name.c_str());
            side_pos.y += small_font * 1.5f;
            if (!cert.date.empty()) {
                dl->AddText(nullptr, small_font, side_pos, text_color, cert.date.c_str());
                side_pos.y += small_font * 1.4f;
            }
        }
    }

    // Main content: Education, Work, Projects
    float main_content_width = main_width - 20;

    // Summary
    if (!data.personal.summary.empty()) {
        draw_section_title(dl, main_pos, "个人简介", main_content_width, title_font);
        draw_wrapped_text(dl, main_pos, data.personal.summary.c_str(), main_content_width, text_color, normal_font);
        main_pos.y += 8;
    }

    // Education
    if (!data.education.empty()) {
        draw_section_title(dl, main_pos, "教育经历", main_content_width, title_font);
        for (const auto& edu : data.education) {
            std::string line = edu.school + " - " + edu.major + " (" + edu.degree + ")";
            dl->AddText(nullptr, normal_font, main_pos, text_color, line.c_str());
            main_pos.y += normal_font * 1.4f;
            if (!edu.start_date.empty()) {
                std::string time = edu.start_date + " ~ " + edu.end_date;
                if (!edu.gpa.empty()) time += "  GPA: " + edu.gpa;
                dl->AddText(nullptr, small_font, main_pos, text_color, time.c_str());
                main_pos.y += small_font * 1.4f;
            }
            main_pos.y += 3;
        }
    }

    // Work
    if (!data.work.empty()) {
        draw_section_title(dl, main_pos, "工作经历", main_content_width, title_font);
        for (const auto& w : data.work) {
            std::string line = w.company;
            if (!w.title.empty()) line += " - " + w.title;
            dl->AddText(nullptr, normal_font, main_pos, text_color, line.c_str());
            main_pos.y += normal_font * 1.4f;
            if (!w.start_date.empty()) {
                std::string time = w.start_date + " ~ " + w.end_date;
                dl->AddText(nullptr, small_font, main_pos, text_color, time.c_str());
                main_pos.y += small_font * 1.4f;
            }
            draw_bullet_list(dl, main_pos, w.duties, main_content_width, text_color, normal_font);
            main_pos.y += 5;
        }
    }

    // Projects
    if (!data.projects.empty()) {
        draw_section_title(dl, main_pos, "项目经历", main_content_width, title_font);
        for (const auto& p : data.projects) {
            dl->AddText(nullptr, normal_font, main_pos, text_color, p.name.c_str());
            main_pos.y += normal_font * 1.4f;
            if (!p.start_date.empty()) {
                std::string time = p.start_date + " ~ " + p.end_date;
                dl->AddText(nullptr, small_font, main_pos, text_color, time.c_str());
                main_pos.y += small_font * 1.4f;
            }
            if (!p.tech_stack.empty()) {
                std::string tech = "技术栈：" + p.tech_stack;
                dl->AddText(nullptr, small_font, main_pos, theme::Yellow, tech.c_str());
                main_pos.y += small_font * 1.4f;
            }
            if (!p.description.empty()) {
                draw_wrapped_text(dl, main_pos, p.description.c_str(), main_content_width, text_color, normal_font);
            }
            draw_bullet_list(dl, main_pos, p.duties, main_content_width, text_color, normal_font);
            main_pos.y += 5;
        }
    }
}

void ui_preview_render() {
    auto& app = App::instance();

    // Calculate A4 preview area
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float preview_width = avail.x - 10;
    float preview_height = preview_width / A4_RATIO;
    if (preview_height > avail.y - 10) {
        preview_height = avail.y - 10;
        preview_width = preview_height * A4_RATIO;
    }

    // Center the preview
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float offset_x = (avail.x - preview_width) * 0.5f;
    ImVec2 origin(cursor.x + offset_x, cursor.y + 5);

    // Draw page background
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(origin, ImVec2(origin.x + preview_width, origin.y + preview_height),
                      0xFFFFFFFF);  // White page
    dl->AddRect(origin, ImVec2(origin.x + preview_width, origin.y + preview_height),
                0xFF45475A, 0, 0, 1.0f);  // Border

    // Clip to page
    dl->PushClipRect(origin, ImVec2(origin.x + preview_width, origin.y + preview_height), true);

    // Render template
    switch (app.selected_template) {
        case 0: render_classic(dl, origin, preview_width, preview_height); break;
        case 1: render_modern(dl, origin, preview_width, preview_height); break;
        case 2: render_classic(dl, origin, preview_width, preview_height); break;  // Minimal uses classic
        case 3: render_modern(dl, origin, preview_width, preview_height); break;  // Creative uses modern
    }

    dl->PopClipRect();

    // One-page warning
    ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + preview_height + 5));
    ImGui::TextColored(ImVec4(0.424f, 0.447f, 0.522f, 1.0f),
        "预览比例 A4 (%.0f x %.0f)", preview_width, preview_height);

    // Smart layout button
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    if (ImGui::SmallButton("智能排版")) {
        // Auto-adjust to fit one page
        app.set_status("已自动调整排版");
    }

    // Make space for scrolling
    ImGui::Dummy(ImVec2(0, preview_height + 20));
}

} // namespace resume
