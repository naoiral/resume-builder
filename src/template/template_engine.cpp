#include "template_engine.h"

namespace resume {

const char* get_template_name(TemplateType type) {
    switch (type) {
        case TemplateType::Classic: return "经典模板";
        case TemplateType::Modern: return "技术岗模板";
        case TemplateType::Minimal: return "实习模板";
        case TemplateType::Creative: return "社招模板";
    }
    return "经典模板";
}

// Estimate content height in "lines" for one-page check
static int estimate_content_lines(const ResumeData& data) {
    int lines = 0;

    // Name + contact + objective
    lines += 5;

    // Summary
    if (!data.personal.summary.empty()) {
        lines += 2 + (int)(data.personal.summary.length() / 40);
    }

    // Education
    for (const auto& e : data.education) {
        lines += 3;
        if (!e.courses.empty()) lines += 1 + (int)(e.courses.length() / 50);
    }

    // Work
    for (const auto& w : data.work) {
        lines += 3 + (int)w.duties.size() * 2;
    }

    // Projects
    for (const auto& p : data.projects) {
        lines += 3 + (int)p.duties.size() * 2;
        if (!p.description.empty()) lines += 1 + (int)(p.description.length() / 50);
    }

    // Skills
    for (const auto& s : data.skills) {
        lines += 2;
    }

    // Certificates
    for (const auto& c : data.certificates) {
        lines += 1;
    }

    return lines;
}

bool fits_one_page(const ResumeData& data, TemplateType type) {
    int lines = estimate_content_lines(data);
    // A4 with standard margins can fit ~55-60 lines at 11pt
    return lines <= 58;
}

float get_auto_fit_scale(const ResumeData& data, TemplateType type) {
    int lines = estimate_content_lines(data);
    if (lines <= 58) return 1.0f;
    // Scale down to fit
    float scale = 58.0f / (float)lines;
    if (scale < 0.6f) scale = 0.6f;  // Minimum readable size
    return scale;
}

} // namespace resume
