#ifndef TEXT_RENDERER_HPP_
#define TEXT_RENDERER_HPP_

#include <SDL3/SDL_render.h>
#include <string>
#include <string_view>
#include <glm/vec2.hpp>
#include "../utils/math.hpp"

struct TTF_TextEngine;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {

    class Camera;

    class TextRenderer final {
    public:
        TextRenderer(
            SDL_Renderer* sdl_renderer,
            engine::resource::ResourceManager* resource_manager
        );

        ~TextRenderer();

        void close();

        void drawUIText(
            std::string_view text,
            std::string_view font_id,
            int font_size,
            const glm::vec2& position,
            const engine::utils::FColor& color = {1.0f, 1.0f, 1.0f, 1.0f}
        );

        void drawText(
            const Camera& camera,
            std::string_view text,
            std::string_view font_id,
            int font_size,
            const glm::vec2& position,
            const engine::utils::FColor& color = {1.0f, 1.0f, 1.0f, 1.0f}
        );

        glm::vec2 getTextSize(std::string_view text, std::string_view font_id, int font_size);

        TextRenderer(const TextRenderer&) = delete;
        TextRenderer& operator=(const TextRenderer&) = delete;
        TextRenderer(TextRenderer&&) = delete;
        TextRenderer& operator=(TextRenderer&&) = delete;

    private:
        SDL_Renderer* sdl_renderer_ = nullptr;
        engine::resource::ResourceManager* resource_manager_ = nullptr;
        TTF_TextEngine* text_engine_ = nullptr;

    };

} // namespace engine::render


#endif // TEXT_RENDERER_HPP_