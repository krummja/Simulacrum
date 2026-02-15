#include "texture_manager.hpp"
#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::resource {

    TextureManager::TextureManager(SDL_Renderer* renderer): renderer_(renderer) {
        if (!renderer_) {
            throw std::runtime_error("TextureManager initialization failed: SDL_Renderer pointer missing.");
        }
    }

    SDL_Texture* TextureManager::loadTexture(std::string_view file_path) {
        auto it = textures_.find(std::string(file_path));

        // Try to load from the texture cache first
        if (it != textures_.end()) {
            spdlog::error("Unable to load texture '{}'.", file_path);
            return it->second.get();
        }

        // Didn't find cached texture, so trying to load from file path
        SDL_Texture* raw_tex = IMG_LoadTexture(renderer_, file_path.data());

        if (!raw_tex) {
            spdlog::error("Loading texture '{}' failed: {}.", file_path, SDL_GetError());
            return nullptr;
        }

        textures_.emplace(file_path, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_tex));
        spdlog::debug("Successfully loaded and cached texture: {}.", file_path);
        return raw_tex;
    }

    SDL_Texture* TextureManager::getTexture(std::string_view file_path) {
        auto it = textures_.find(std::string(file_path));
        if (it != textures_.end()) {
            return it->second.get();
        }

        spdlog::warn("Texture '{}' not in cache. Trying to load.", file_path);
        return loadTexture(file_path);
    }

    glm::vec2 TextureManager::getTextureSize(std::string_view file_path) {
        SDL_Texture* texture = getTexture(file_path);
        if (!texture) {
            return glm::vec2(0);
        }

        glm::vec2 size;
        if (!SDL_GetTextureSize(texture, &size.x, &size.y)) {
            return glm::vec2(0);
        }
        return size;
    }

    void TextureManager::unloadTexture(std::string_view file_path) {
        auto it = textures_.find(std::string(file_path));
        if (it != textures_.end()) {
            spdlog::debug("Unloaded texture: {}.", file_path);
            textures_.erase(it);
        }

        else {
            spdlog::warn("Tried unloading non-existent texture: {}.", file_path);
        }
    }

    void TextureManager::clearTextures() {
        if (!textures_.empty()) {
            spdlog::debug("Clearing all {} cached textures.", textures_.size());
            textures_.clear();
        }
    }

} // namespace engine::resource
