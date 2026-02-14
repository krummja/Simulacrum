#include "texture_manager.hpp"
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

    TextureManager::TextureManager(SDL_Renderer* renderer): renderer_(renderer) {}

    SDL_Texture* TextureManager::loadTexture(std::string_view file_path) {
        return nullptr;
    }

    SDL_Texture* TextureManager::getTexture(std::string_view file_path) {
        return nullptr;
    }

    glm::vec2 TextureManager::getTextureSize(std::string_view file_path) {
        return glm::vec2(0);
    }

    void TextureManager::unloadTexture(std::string_view file_path) {}

    void TextureManager::clearTextures() {}

} // namespace engine::resource
