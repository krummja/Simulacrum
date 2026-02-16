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
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            return;
        }

        auto src_rect = getSpriteSrcRect(sprite);
        if (!src_rect.has_value()) {
            return;
        }

        // glm::vec2 position_screen = camera.worldToScreen(position);
    }

    void Renderer::drawParallax(
        const Camera& camera,
        const Sprite& sprite,
        const glm::vec2& position,
        const glm::vec2& scroll_factor,
        glm::bvec2 repeat,
        const glm::vec2& scale
    ) {
        spdlog::warn("drawParallax is not implemented!");
    }

    void Renderer::drawUISprite(
        const Sprite& sprite,
        const glm::vec2& position,
        const std::optional<glm::vec2>& size
    ) {
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("Could not load sprite texture, ID: {}", sprite.getTextureId());
            return;
        }

        auto src_rect = getSpriteSrcRect(sprite);
        if (!src_rect.has_value()) {
            spdlog::error("Unable to get source rectangle of sprite, ID: {}", sprite.getTextureId());
            return;
        }

        SDL_FRect dest_rect = {position.x, position.y, 0, 0};

        if (size.has_value()) {
            dest_rect.w = size.value().x;
            dest_rect.h = size.value().y;
        }

        else {
            dest_rect.w = src_rect.value().w;
            dest_rect.h = src_rect.value().h;
        }

        if (!SDL_RenderTextureRotated(
            renderer_,
            texture,
            &src_rect.value(),
            &dest_rect,
            0.0,
            nullptr,
            sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
        )) {
            spdlog::error("Could not render UI Sprite (ID: {}): {}", sprite.getTextureId(), SDL_GetError());
        }
    }

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
        SDL_Texture* texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            return std::nullopt;
        }

        auto src_rect = sprite.getSourceRect();

        if (src_rect.has_value()) {
            if (src_rect.value().w <= 0 || src_rect.value().h <= 0) {
                return std::nullopt;
            }
            return src_rect;
        }

        else {
            SDL_FRect result = {0, 0, 0, 0};
            if (!SDL_GetTextureSize(texture, &result.w, &result.h)) {
                return std::nullopt;
            }

            return result;
        }

        return std::nullopt;
    }

    bool Renderer::isRectInViewport(const Camera& camera, const SDL_FRect &rect) {
        glm::vec2 viewport_size = camera.getViewportSize();
        return rect.x + rect.w >= 0 && rect.x <= viewport_size.x &&
               rect.x + rect.h >= 0 && rect.y <= viewport_size.y;
    }

} // namespace engine::render
