#include "font_manager.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

    FontManager::FontManager() {}

    FontManager::~FontManager() {}

    TTF_Font* FontManager::loadFont(std::string_view file_path, int point_size) {
        return nullptr;
    }

    TTF_Font* FontManager::getFont(std::string_view file_path, int point_size) {
        return nullptr;
    }

    void FontManager::unloadFont(std::string_view file_path, int point_size) {}

    void FontManager::clearFonts() {}

} // namespace engine::resource
