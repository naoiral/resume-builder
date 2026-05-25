#pragma once
#include "resume_model.h"
#include <string>

namespace resume {

bool save_to_file(const ResumeData& data, const std::string& path);
bool load_from_file(ResumeData& data, const std::string& path);
std::string to_json_string(const ResumeData& data);
bool from_json_string(ResumeData& data, const std::string& json);

} // namespace resume
