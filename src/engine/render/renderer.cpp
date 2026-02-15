#include "renderer.hpp"
#include "../resource/resource_manager.hpp"
#include "camera.hpp"
#include "sprite.hpp"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace engine::render {

    Renderer::Renderer(
        SDL_Renderer* sdl_renderer,
        engine::resource::ResourceManager* resource_manager
    )
        : renderer_(sdl_renderer)
        , resource_manager_(resource_manager)
    {
        if (!renderer_) {
            throw std::runtime_error(
                "Renderer failed to load: The provided SDL_Renderer pointer is empty."
            );
        }

        if (!resource_manager_) {
            throw std::runtime_error(
                "Renderer failed to load: The provided ResourceManager pointer is empty."
            );
        }

        setDrawColor(255, 0, 0, 255);
    }

    void Renderer::drawSprite(
        const Camera& camera,
        const Sprite& sprite,
        const glm::vec2& position,
        const glm::vec2& scale,
        double angle
    ) {
        // auto texture = resource_manager_->getTexture(sprite.getTextureId());
        // if (!texture) {
        //     return;
        // }
    }

    void Renderer::drawParallax() {}
    void Renderer::drawUISprite() {}

    void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        if (!SDL_SetRenderDrawColor(renderer_, r, g, b, a)) {
            spdlog::error("Setting render draw color failed: {}", SDL_GetError());
        }
    }

    void Renderer::setDrawColorFloat(float r, float g, float b, float a) {
        if (!SDL_SetRenderDrawColorFloat(renderer_, r, g, b, a)) {
            spdlog::error("Setting render draw color failed: {}", SDL_GetError());
        }
    }

    void Renderer::clearScreen() {
        if (!SDL_RenderClear(renderer_)) {
            spdlog::error("Clearing renderer failed: {}", SDL_GetError());
        }
    }

    void Renderer::drawUIFilledRect(const engine::utils::Rect &rect, const engine::utils::FColor &color) {
        setDrawColorFloat(color.r, color.g, color.b, color.a);
        if (!SDL_RenderFillRect(renderer_, reinterpret_cast<const SDL_FRect*>(&rect))) {
            spdlog::error("Drawing filled rectangle failed: {}", SDL_GetError());
        }
        setDrawColorFloat(0, 0, 0, 1.0f);
    }

    void Renderer::present() {
        SDL_RenderPresent(renderer_);
    }

    std::optional<SDL_FRect> Renderer::getSpriteSrcRect(const Sprite &sprite) {
        return std::nullopt;
    }

    bool Renderer::isRectInViewport(const Camera& camera, const SDL_FRect &rect) {
        return true;
    }

} // namespace engine::render
