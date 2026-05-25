#pragma once
#include <string>

namespace resume {

void ui_preview_render();

// Photo texture management
unsigned int load_texture(const std::string& path);
void destroy_textures();

} // namespace resume
