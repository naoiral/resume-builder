#pragma once
#include "data/resume_model.h"
#include <string>

namespace resume {

// Template types
enum class TemplateType { Classic, Modern, Minimal, Creative };

// Get template name
const char* get_template_name(TemplateType type);

// Check if content fits one page (estimated)
bool fits_one_page(const ResumeData& data, TemplateType type);

// Get auto-fit font scale (0.5 - 1.0)
float get_auto_fit_scale(const ResumeData& data, TemplateType type);

} // namespace resume
