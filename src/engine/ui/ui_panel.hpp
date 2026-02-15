#ifndef UI_PANEL_HPP_
#define UI_PANEL_HPP_

#include "ui_element.hpp"
#include <optional>
#include "../utils/math.hpp"

namespace engine::ui {

    class UIPanel final : public UIElement {
        std::optional<engine::utils::FColor> background_color_;

    public:
        explicit UIPanel(
            glm::vec2 position = {0.0f, 0.0f},
            glm::vec2 size = {0.0f, 0.0f},
            std::optional<engine::utils::FColor> background_color = std::nullopt
        );

        void setBackgroundColor(std::optional<engine::utils::FColor> background_color) {
            background_color_ = std::move(background_color);
        }

        const std::optional<engine::utils::FColor>& getBackgroundColor() const {
            return background_color_;
        }

        void render(engine::core::Context& context) override;
    };

} // namespace engine::ui

#endif // UI_PANEL_HPP_