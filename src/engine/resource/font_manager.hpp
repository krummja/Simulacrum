#ifndef FONT_MANAGER_HPP_
#define FONT_MANAGER_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <functional>
#include <SDL3_ttf/SDL_ttf.h>

namespace engine::resource {

    using FontKey = std::pair<std::string, int>;

    struct FontKeyHash {
        std::size_t operator()(const FontKey& key) const {
            std::hash<std::string> string_hasher;
            std::hash<int> int_hasher;
            return string_hasher(key.first) ^ int_hasher(key.second);
        }
    };

    class FontManager final {
        friend class ResourceManager;

    public:
        FontManager();
        ~FontManager();

        FontManager(const FontManager&) = delete;
        FontManager& operator=(const FontManager&) = delete;
        FontManager(FontManager&&) = delete;
        FontManager& operator=(FontManager&&) = delete;

    private:
        struct SDLFontDeleter {
            void operator()(TTF_Font* font) const {
                if (font) {
                    TTF_CloseFont(font);
                }
            }
        };

        std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHash> fonts_;

        TTF_Font* loadFont(std::string_view file_path, int point_size);
        TTF_Font* getFont(std::string_view file_path, int point_size);
        void unloadFont(std::string_view file_path, int point_size);
        void clearFonts();

    };

} // namespace engine::resource

#endif // FONT_MANAGER_HPP_