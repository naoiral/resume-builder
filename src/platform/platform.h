#pragma once
#include <string>

namespace resume {
namespace platform {

// File dialogs
std::string open_file_dialog();
std::string save_file_dialog(const char* default_name = nullptr);
std::string save_pdf_dialog();

// Open folder in explorer
void open_in_explorer(const std::string& path);

} // namespace platform
} // namespace resume
