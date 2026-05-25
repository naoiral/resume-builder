#pragma once
#include "data/resume_model.h"
#include <string>

namespace resume {

// Export resume to PDF file
bool export_resume_pdf(const ResumeData& data, const std::string& filepath);

} // namespace resume
