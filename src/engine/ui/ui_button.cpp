#include "ui_button.hpp"
#include "state/ui_normal_state.hpp"
#include <spdlog/spdlog.h>

namespace engine::ui {

    UIButton::UIButton(
        engine::core::Context& context,
        std::string_view normal_sprite_id,
        std::string_view hover_sprite_id,
        std::string_view pressed_sprite_id,
        glm::vec2 position,
        glm::vec2 size,
        std::function<void()> callback
    )
        : UIInteractive(context, std::move(position), std::move(size))
        , callback_(std::move(callback))
    {
        addSprite("normal", std::make_unique<engine::render::Sprite>(normal_sprite_id));
        addSprite("hover", std::make_unique<engine::render::Sprite>(hover_sprite_id));
        addSprite("pressed", std::make_unique<engine::render::Sprite>(pressed_sprite_id));

        setState(std::make_unique<engine::ui::state::UINormalState>(this));
    }

    void UIButton::clicked() {
        if (callback_) callback_();
    }

} // namespace engine::ui
