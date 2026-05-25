#include "pdf_export.h"
#include <hpdf.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace resume {

// Page dimensions in points (1mm = 2.835 points)
static constexpr float PAGE_W = 210.0f * 2.835f;  // A4 width
static constexpr float PAGE_H = 297.0f * 2.835f;  // A4 height
static constexpr float MARGIN_TOP = 20.0f * 2.835f;
static constexpr float MARGIN_BOTTOM = 20.0f * 2.835f;
static constexpr float MARGIN_LEFT = 15.0f * 2.835f;
static constexpr float MARGIN_RIGHT = 15.0f * 2.835f;
static constexpr float CONTENT_W = PAGE_W - MARGIN_LEFT - MARGIN_RIGHT;

static constexpr float FONT_NAME = 18.0f;
static constexpr float FONT_SECTION = 14.0f;
static constexpr float FONT_NORMAL = 11.0f;
static constexpr float FONT_SMALL = 10.0f;
static constexpr float LINE_SPACING = 1.5f;

static const HPDF_RGBColor COLOR_TEXT = {0.2f, 0.2f, 0.2f};
static const HPDF_RGBColor COLOR_SECTION = {0.0f, 0.4f, 0.8f};
static const HPDF_RGBColor COLOR_GRAY = {0.5f, 0.5f, 0.5f};
static const HPDF_RGBColor COLOR_BLACK = {0.0f, 0.0f, 0.0f};

struct PdfContext {
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Font font_regular;
    HPDF_Font font_bold;
    float y;
    int page_num;
    bool use_cid;

    PdfContext() : pdf(nullptr), page(nullptr), font_regular(nullptr),
                   font_bold(nullptr), y(0), page_num(0), use_cid(false) {}
};

static void add_page(PdfContext& ctx);
static bool check_page_break(PdfContext& ctx, float needed);

static void add_page(PdfContext& ctx) {
    ctx.page = HPDF_AddPage(ctx.pdf);
    HPDF_Page_SetSize(ctx.page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    // Explicitly set exact A4 dimensions in points (210mm x 297mm)
    HPDF_Page_SetWidth(ctx.page, PAGE_W);
    HPDF_Page_SetHeight(ctx.page, PAGE_H);
    ctx.y = PAGE_H - MARGIN_TOP;
    ctx.page_num++;
}

static bool check_page_break(PdfContext& ctx, float needed) {
    if (ctx.y - needed < MARGIN_BOTTOM) {
        add_page(ctx);
        return true;
    }
    return false;
}

// Draw text - passes UTF-8 directly when using UTF-8 CID encoding
static float draw_text(PdfContext& ctx, HPDF_Font font, float size,
                        const char* text, HPDF_RGBColor color,
                        float max_width = CONTENT_W) {
    if (!text || !text[0]) return 0;

    HPDF_Page_SetFontAndSize(ctx.page, font, size);
    HPDF_Page_SetRGBFill(ctx.page, color.r, color.g, color.b);

    float line_height = size * LINE_SPACING;
    float total_height = 0;
    const char* p = text;
    std::string current_line;

    while (*p) {
        while (*p && *p != '\n') {
            current_line += *p;
            p++;

            float tw = HPDF_Page_TextWidth(ctx.page, current_line.c_str());
            if (tw > max_width) {
                // Back up one UTF-8 character
                current_line.pop_back();
                p--;
                while (p > text && ((*p) & 0xC0) == 0x80) {
                    current_line.pop_back();
                    p--;
                }

                HPDF_Page_BeginText(ctx.page);
                HPDF_Page_TextOut(ctx.page, MARGIN_LEFT, ctx.y, current_line.c_str());
                HPDF_Page_EndText(ctx.page);

                ctx.y -= line_height;
                total_height += line_height;
                current_line.clear();
                check_page_break(ctx, line_height * 3);
            }
        }

        if (!current_line.empty()) {
            HPDF_Page_BeginText(ctx.page);
            HPDF_Page_TextOut(ctx.page, MARGIN_LEFT, ctx.y, current_line.c_str());
            HPDF_Page_EndText(ctx.page);

            ctx.y -= line_height;
            total_height += line_height;
            current_line.clear();
        }

        if (*p == '\n') {
            ctx.y -= line_height;
            total_height += line_height;
            p++;
        }
    }

    return total_height;
}

static void draw_section_title(PdfContext& ctx, const char* title) {
    float line_height = FONT_SECTION * LINE_SPACING;
    check_page_break(ctx, line_height + 20);

    draw_text(ctx, ctx.font_bold, FONT_SECTION, title, COLOR_SECTION);
    // ctx.y is now at the baseline of the next content line after title

    // Draw line centered between title bottom and content top
    float title_bottom = ctx.y + line_height;  // bottom of title text
    float content_top = ctx.y - 8;             // where content will start (8pt gap below line)
    float line_y = (title_bottom + content_top) * 0.5f;

    HPDF_Page_SetLineWidth(ctx.page, 1.5f);
    HPDF_Page_SetRGBStroke(ctx.page, COLOR_SECTION.r, COLOR_SECTION.g, COLOR_SECTION.b);
    HPDF_Page_MoveTo(ctx.page, MARGIN_LEFT, line_y);
    HPDF_Page_LineTo(ctx.page, MARGIN_LEFT + CONTENT_W, line_y);
    HPDF_Page_Stroke(ctx.page);

    ctx.y -= 8;
}

static void draw_bullets(PdfContext& ctx, const std::vector<std::string>& items) {
    for (const auto& item : items) {
        if (item.empty()) continue;
        std::string bullet = "  *  " + item;
        draw_text(ctx, ctx.font_regular, FONT_NORMAL, bullet.c_str(), COLOR_TEXT, CONTENT_W - 15);
        ctx.y -= 2;
    }
}

static void draw_photo(PdfContext& ctx, const std::string& photo_path) {
    if (photo_path.empty()) return;

    HPDF_Image image = nullptr;

    // Try loading as PNG first, then JPEG
    std::string ext = photo_path;
    for (auto& c : ext) c = (char)tolower(c);

    if (ext.find(".png") != std::string::npos) {
        image = HPDF_LoadPngImageFromFile(ctx.pdf, photo_path.c_str());
    } else if (ext.find(".jpg") != std::string::npos || ext.find(".jpeg") != std::string::npos) {
        image = HPDF_LoadJpegImageFromFile(ctx.pdf, photo_path.c_str());
    }

    if (!image) return;

    // Photo size: 3cm x 4cm (passport photo ratio)
    float photo_w = 30.0f * 2.835f;
    float photo_h = 40.0f * 2.835f;

    // Position at top-right corner
    float photo_x = PAGE_W - MARGIN_RIGHT - photo_w;
    float photo_y = ctx.y - photo_h + FONT_NAME;

    // Draw border around photo
    HPDF_Page_SetLineWidth(ctx.page, 0.5f);
    HPDF_Page_SetRGBStroke(ctx.page, COLOR_GRAY.r, COLOR_GRAY.g, COLOR_GRAY.b);
    HPDF_Page_Rectangle(ctx.page, photo_x - 1, photo_y - 1, photo_w + 2, photo_h + 2);
    HPDF_Page_Stroke(ctx.page);

    // Draw image
    HPDF_Page_DrawImage(ctx.page, image, photo_x, photo_y, photo_w, photo_h);
}

static void export_classic(PdfContext& ctx, const ResumeData& data) {
    // Draw photo first (top-right corner)
    draw_photo(ctx, data.personal.photo_path);

    if (!data.personal.name.empty()) {
        draw_text(ctx, ctx.font_bold, FONT_NAME, data.personal.name.c_str(), COLOR_BLACK);
        ctx.y -= 5;
    }

    std::string contact;
    if (!data.personal.phone.empty()) contact += data.personal.phone + "  |  ";
    if (!data.personal.email.empty()) contact += data.personal.email;
    if (!contact.empty()) {
        draw_text(ctx, ctx.font_regular, FONT_SMALL, contact.c_str(), COLOR_GRAY);
        ctx.y -= 3;
    }

    if (!data.personal.objective.empty()) {
        std::string obj = std::string(u8"求职意向：") + data.personal.objective;
        draw_text(ctx, ctx.font_regular, FONT_NORMAL, obj.c_str(), COLOR_TEXT);
        ctx.y -= 3;
    }

    ctx.y -= 5;

    if (!data.personal.summary.empty()) {
        draw_section_title(ctx, u8"个人简介");
        draw_text(ctx, ctx.font_regular, FONT_NORMAL, data.personal.summary.c_str(), COLOR_TEXT);
        ctx.y -= 8;
    }

    if (!data.education.empty()) {
        draw_section_title(ctx, u8"教育经历");
        for (const auto& edu : data.education) {
            std::string header = edu.school + "  " + edu.major + "  " + edu.degree;
            draw_text(ctx, ctx.font_bold, FONT_NORMAL, header.c_str(), COLOR_TEXT);

            if (!edu.start_date.empty()) {
                std::string time = edu.start_date + " - " + edu.end_date;
                if (!edu.gpa.empty()) time += "  GPA: " + edu.gpa;
                draw_text(ctx, ctx.font_regular, FONT_SMALL, time.c_str(), COLOR_GRAY);
            }

            if (!edu.courses.empty()) {
                std::string courses = std::string(u8"主修课程：") + edu.courses;
                draw_text(ctx, ctx.font_regular, FONT_SMALL, courses.c_str(), COLOR_GRAY);
            }
            ctx.y -= 6;
        }
    }

    if (!data.work.empty()) {
        draw_section_title(ctx, u8"工作经历");
        for (const auto& w : data.work) {
            std::string header = w.company;
            if (!w.title.empty()) header += "  " + w.title;
            draw_text(ctx, ctx.font_bold, FONT_NORMAL, header.c_str(), COLOR_TEXT);

            if (!w.start_date.empty()) {
                std::string time = w.start_date + " - " + w.end_date;
                draw_text(ctx, ctx.font_regular, FONT_SMALL, time.c_str(), COLOR_GRAY);
            }

            draw_bullets(ctx, w.duties);
            ctx.y -= 6;
        }
    }

    if (!data.projects.empty()) {
        draw_section_title(ctx, u8"项目经历");
        for (const auto& p : data.projects) {
            draw_text(ctx, ctx.font_bold, FONT_NORMAL, p.name.c_str(), COLOR_TEXT);

            if (!p.start_date.empty()) {
                std::string time = p.start_date + " - " + p.end_date;
                if (!p.tech_stack.empty()) time += std::string(u8"  技术栈: ") + p.tech_stack;
                draw_text(ctx, ctx.font_regular, FONT_SMALL, time.c_str(), COLOR_GRAY);
            }

            if (!p.description.empty()) {
                draw_text(ctx, ctx.font_regular, FONT_NORMAL, p.description.c_str(), COLOR_TEXT);
            }
            draw_bullets(ctx, p.duties);
            ctx.y -= 6;
        }
    }

    if (!data.skills.empty()) {
        draw_section_title(ctx, u8"技能特长");
        for (const auto& cat : data.skills) {
            std::string line = cat.category + u8"：";
            for (size_t i = 0; i < cat.items.size(); i++) {
                if (i > 0) line += u8"、";
                line += cat.items[i].name + "(" + skill_level_str(cat.items[i].level) + ")";
            }
            draw_text(ctx, ctx.font_regular, FONT_NORMAL, line.c_str(), COLOR_TEXT);
            ctx.y -= 3;
        }
        ctx.y -= 5;
    }

    if (!data.certificates.empty()) {
        draw_section_title(ctx, u8"证书奖项");
        for (const auto& cert : data.certificates) {
            std::string line = cert.name;
            if (!cert.date.empty()) line += "  (" + cert.date + ")";
            draw_text(ctx, ctx.font_regular, FONT_NORMAL, line.c_str(), COLOR_TEXT);
            ctx.y -= 3;
        }
    }
}

static bool g_pdf_error = false;
static void pdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    g_pdf_error = true;
}

bool export_resume_pdf(const ResumeData& data, const std::string& filepath) {
    PdfContext ctx;
    g_pdf_error = false;

#ifdef _WIN32
    // Debug log
    char exe_path_dbg[MAX_PATH];
    GetModuleFileNameA(nullptr, exe_path_dbg, MAX_PATH);
    std::string exe_dir_dbg(exe_path_dbg);
    size_t sl_dbg = exe_dir_dbg.find_last_of("\\/");
    if (sl_dbg != std::string::npos) exe_dir_dbg = exe_dir_dbg.substr(0, sl_dbg + 1);
    FILE* dbg = fopen((exe_dir_dbg + "pdf_debug.log").c_str(), "w");
    if (dbg) {
        fprintf(dbg, "=== PDF Export Debug ===\n");
        fprintf(dbg, "Name: %s\n", data.personal.name.c_str());
        fflush(dbg);
    }
#else
    FILE* dbg = nullptr;
#endif

    ctx.pdf = HPDF_New(pdf_error_handler, nullptr);
    if (!ctx.pdf) {
#ifdef _WIN32
        if (dbg) { fprintf(dbg, "ERROR: HPDF_New failed\n"); fclose(dbg); }
        MessageBoxA(nullptr, "Failed to create PDF document", "Error", MB_OK | MB_ICONERROR);
#endif
        return false;
    }

    HPDF_SetCompressionMode(ctx.pdf, HPDF_COMP_ALL);

    // Register UTF-8 encoding for Chinese support
    HPDF_UseUTFEncodings(ctx.pdf);

    // Build font path relative to executable location
    std::string font_path;
#ifdef _WIN32
    char exe_path[MAX_PATH];
    GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
    std::string exe_dir(exe_path);
    size_t last_slash = exe_dir.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        exe_dir = exe_dir.substr(0, last_slash + 1);
    }
    font_path = exe_dir + "fonts\\SourceHanSansCN-Regular.ttf";
#else
    font_path = "./fonts/SourceHanSansCN-Regular.ttf";
#endif

    // Load font with UTF-8 encoding
    bool font_loaded = false;
    FILE* f = fopen(font_path.c_str(), "rb");
    if (dbg) { fprintf(dbg, "Font file exists: %s\n", f ? "YES" : "NO"); fflush(dbg); }
    if (f) {
        fclose(f);
        HPDF_ResetError(ctx.pdf);
        g_pdf_error = false;
        const char* font_name = HPDF_LoadTTFontFromFile(ctx.pdf, font_path.c_str(), HPDF_TRUE);
        if (dbg) { fprintf(dbg, "LoadTTFont: name=%s error=%d\n", font_name ? font_name : "NULL", (int)g_pdf_error); fflush(dbg); }
        if (font_name && !g_pdf_error) {
            HPDF_ResetError(ctx.pdf);
            g_pdf_error = false;
            HPDF_Font font = HPDF_GetFont(ctx.pdf, font_name, "UTF-8");
            if (dbg) { fprintf(dbg, "GetFont UTF-8: %s error=%d\n", font ? "OK" : "NULL", (int)g_pdf_error); fflush(dbg); }
            if (font && !g_pdf_error) {
                ctx.font_regular = font;
                ctx.font_bold = font;
                ctx.use_cid = true;
                font_loaded = true;
            }
        }
    }

    if (dbg) { fprintf(dbg, "Font loaded: %s\n", font_loaded ? "YES" : "NO"); fflush(dbg); }

    if (!font_loaded) {
        HPDF_Free(ctx.pdf);
        if (dbg) fclose(dbg);
#ifdef _WIN32
        std::string msg = "Font file not found or failed to load:\n" + font_path +
                          "\n\nPlease ensure the fonts folder exists next to the executable.";
        MessageBoxA(nullptr, msg.c_str(), "Font Error", MB_OK | MB_ICONERROR);
#endif
        return false;
    }

    add_page(ctx);
    if (dbg) { fprintf(dbg, "Page added, y=%.1f\n", ctx.y); fflush(dbg); }
    export_classic(ctx, data);
    if (dbg) { fprintf(dbg, "Content rendered, y=%.1f\n", ctx.y); fflush(dbg); }

    g_pdf_error = false;
    HPDF_STATUS status = HPDF_SaveToFile(ctx.pdf, filepath.c_str());
    if (dbg) { fprintf(dbg, "Save: status=%04X error=%d\n", (unsigned)status, (int)g_pdf_error); fflush(dbg); }
    HPDF_Free(ctx.pdf);

    if (status != HPDF_OK || g_pdf_error) {
#ifdef _WIN32
        if (dbg) fclose(dbg);
        MessageBoxA(nullptr, "PDF file save failed", "Export Error", MB_OK | MB_ICONERROR);
#endif
        return false;
    }

#ifdef _WIN32
    if (dbg) { fprintf(dbg, "SUCCESS\n"); fclose(dbg); }
    MessageBoxA(nullptr, "PDF exported successfully!", "Success", MB_OK | MB_ICONINFORMATION);
#endif

    return true;
}

} // namespace resume
