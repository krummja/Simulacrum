#ifndef RENDERER_HPP_
#define RENDERER_HPP_
#include "sprite.hpp"
#include <string>
#include <optional>
#include <SDL_stdinc.h>

struct SDL_Renderer;
struct SDL_FRect;
struct SDL_FColor;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {
    class Camera;

    /// @brief Encapsulating SDL3 rendering operations
    ///
    /// Wraps SDL_Renderer and provides methods to clear the screen, draw sprites, and
    /// render the final image. Initialized at construction time. Depends on a valid
    /// SDL_Renderer and ResourceManager.
    ///
    /// Construction failure will throw an exception.
    class Renderer final {
    public:
        Renderer(
            SDL_Renderer* sdl_renderer,
            engine::resource::ResourceManager* resource_manager
        );

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void drawSprite();
        void drawParallax();
        void drawUISprite();
        void drawUIFilledRect();

        void present();
        void clearScreen();
        void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
        void setDrawColorFloat(float r, float g, float b, float a = 1.0f);

        SDL_Renderer* getSDLRenderer() const { return renderer_; }

    private:
        /// @brief Non-owned pointer to SDL_Renderer.
        SDL_Renderer* renderer_ = nullptr;

        /// @brief Non-owned pointer to ResourceManager.
        engine::resource::ResourceManager* resource_manager_ = nullptr;

        /// @brief Get the source rectangle of the sprite for specific drawing. If an
        /// error occurs, return `std::nullopt` and skip drawing.
        std::optional<SDL_FRect> getSpriteSrcRect(const Sprite& sprite);

        /// @brief Determine whether the rectangle is in the viewport. Used for viewport
        /// cropping.
        bool isRectInViewport(const Camera& camera, const SDL_FRect& rect);

    };

} // namespace engine::render

#endif // RENDERER_HPP_
