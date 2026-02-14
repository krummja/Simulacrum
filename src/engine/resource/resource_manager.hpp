#ifndef RESOURCE_MANAGER_HPP_
#define RESOURCE_MANAGER_HPP_
#include <memory>
#include <string>
#include <string_view>
#include <glm/glm.hpp>

struct SDL_Renderer;
struct SDL_Texture;
struct Mix_Chunk;
struct Mix_Music;
struct TTF_Font;

namespace engine::resource {

    class TextureManager;
    class AudioManager;
    class FontManager;

    class ResourceManager final {
    public:
        explicit ResourceManager(SDL_Renderer* renderer);

        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;

        void clear();

        SDL_Texture* loadTexture(std::string_view file_path);
        SDL_Texture* getTexture(std::string_view file_path);
        void unloadTexture(std::string_view file_path);
        glm::vec2 getTextureSize(std::string_view file_path);
        void clearTextures();

        Mix_Chunk* loadSound(std::string_view file_path);
        Mix_Chunk* getSound(std::string_view file_path);
        void unloadSound(std::string_view file_path);
        void clearSounds();

        Mix_Music* loadMusic(std::string_view file_path);
        Mix_Music* getMusic(std::string_view file_path);
        void unloadMusic(std::string_view file_path);
        void clearMusic();

        TTF_Font* loadFont(std::string_view file_path, int point_size);
        TTF_Font* getFont(std::string_view file_path, int point_size);
        void unloadFont(std::string_view file_path, int point_size);
        void clearFonts();

    private:
        std::unique_ptr<TextureManager> texture_manager_;
        std::unique_ptr<AudioManager> audio_manager_;
        std::unique_ptr<FontManager> font_manager_;

    };

} // engine::resource

#endif RESOURCE_MANAGER_HPP_