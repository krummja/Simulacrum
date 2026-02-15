#include "font_manager.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

    FontManager::FontManager() {
        if (!TTF_WasInit() && !TTF_Init()) {
            throw std::runtime_error("FontManager error: TTF_Init failed: " + std::string(SDL_GetError()));
        }
    }

    FontManager::~FontManager() {
        if (!fonts_.empty()) {
            spdlog::debug("FontManager not empty, calling `clearFonts` to handle cleanup logic.");
            clearFonts();
        }

        TTF_Quit();
    }

    TTF_Font* FontManager::loadFont(std::string_view file_path, int point_size) {
        if (point_size <= 0) {
            spdlog::error("Unable to load font '{}': invalid point size {}.", file_path, point_size);
            return nullptr;
        }

        FontKey key = {std::string(file_path), point_size};

        auto it = fonts_.find(key);
        if (it != fonts_.end()) {
            return it->second.get();
        }

        spdlog::debug("Loaded font: {} ({}pt).", file_path, point_size);
        TTF_Font* raw_font = TTF_OpenFont(file_path.data(), point_size);
        if (!raw_font) {
            spdlog::error("Loading font '{}' ({}pt) failed: {}.", file_path, point_size, SDL_GetError());
            return nullptr;
        }

        fonts_.emplace(key, std::unique_ptr<TTF_Font, SDLFontDeleter>(raw_font));
        spdlog::debug("Successfully loaded and cached font: {} ({}pt).", file_path, point_size);
        return raw_font;
    }

    TTF_Font* FontManager::getFont(std::string_view file_path, int point_size) {
        FontKey key = {std::string(file_path), point_size};
        auto it = fonts_.find(key);
        if (it != fonts_.end()) {
            return it->second.get();
        }

        spdlog::warn("Font '{}' ({}pt) not in cache. Trying to load.", file_path, point_size);
        return loadFont(file_path, point_size);
    }

    void FontManager::unloadFont(std::string_view file_path, int point_size) {
        FontKey key = {std::string(file_path), point_size};
        auto it = fonts_.find(key);
        if (it != fonts_.end()) {
            spdlog::debug("Unloaded font: {} ({}pt).", file_path, point_size);
            fonts_.erase(it);
        }

        else {
            spdlog::warn("Tried unloading non-existent font: {} ({}pt).", file_path, point_size);
        }
    }

    void FontManager::clearFonts() {
        if (!fonts_.empty()) {
            spdlog::debug("Clearing all {} cached fonts.", fonts_.size());
            fonts_.clear();
        }
    }

} // namespace engine::resource
