#ifndef UI_BUTTON_HPP_
#define UI_BUTTON_HPP_

#include "ui_interactive.hpp"
#include <functional>
#include <utility>

namespace engine::ui {

    class UIButton final : public UIInteractive {
    public:
        UIButton(
            engine::core::Context& context,
            std::string_view normal_sprite_id,
            std::string_view hover_sprite_id,
            std::string_view pressed_sprite_id,
            glm::vec2 position = {0.0f, 0.0f},
            glm::vec2 size = {0.0f, 0.0f},
            std::function<void()> callback = nullptr
        );

        ~UIButton() override = default;

        void clicked() override;
        void setCallback(std::function<void()> callback) { callback_ = std::move(callback); }
        std::function<void()> getCallback() const { return callback_; }

    private:
        std::function<void()> callback_;

    };

} // namespace engine::ui

#endif // UI_BUTTON_HPP_