#include "ui_panel.hpp"
#include "../core/context.hpp"
#include "../render/renderer.hpp"
#include <SDL3/SDL_pixels.h>
#include <spdlog/spdlog.h>

namespace engine::ui {

    UIPanel::UIPanel(
        glm::vec2 position,
        glm::vec2 size,
        std::optional<engine::utils::FColor> background_color
    ) : UIElement(std::move(position), std::move(size)), background_color_(std::move(background_color))
    {}

    void UIPanel::render(engine::core::Context& context) {
        if (!visible_) return;

        if (background_color_) {
            context.getRenderer().drawUIFilledRect(getBounds(), background_color_.value());
        }

        UIElement::render(context);
    }

} // namespace engine::ui

