#ifndef TEXTURE_MANAGER_HPP_
#define TEXTURE_MANAGER_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>

namespace engine::resource {

    class TextureManager final {
        friend class ResourceManager;

    public:
        explicit TextureManager(SDL_Renderer* renderer);

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;
        TextureManager(TextureManager&&) = delete;
        TextureManager& operator=(TextureManager&&) = delete;

    private:
        struct SDLTextureDeleter {
            void operator()(SDL_Texture* texture) const {
                if (texture) {
                    SDL_DestroyTexture(texture);
                }
            }
        };

        std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_;
        SDL_Renderer* renderer_ = nullptr;

        SDL_Texture* loadTexture(std::string_view file_path);
        SDL_Texture* getTexture(std::string_view file_path);
        glm::vec2 getTextureSize(std::string_view file_path);
        void unloadTexture(std::string_view file_path);
        void clearTextures();

    };

} // namespace engine::resource

#endif // TEXTURE_MANAGER_HPP_