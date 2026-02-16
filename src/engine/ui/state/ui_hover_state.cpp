#include "ui_hover_state.hpp"
#include "ui_normal_state.hpp"
#include "ui_pressed_state.hpp"
#include "../ui_interactive.hpp"
#include "../../input/input_manager.hpp"
#include "../../core/context.hpp"
#include <spdlog/spdlog.h>

namespace engine::ui::state {

    void UIHoverState::enter() {
        owner_->setSprite("hover");
    }

    std::unique_ptr<UIState> UIHoverState::handleInput(engine::core::Context& context) {
        auto& input_manager = context.getInputManager();
        auto mouse_pos = input_manager.getLogicalMousePosition();

        if (!owner_->isPointInside(mouse_pos)) {
            return std::make_unique<UINormalState>(owner_);
        }

        if (input_manager.isActionPressed("MouseLeftClick")) {
            return std::make_unique<UIPressedState>(owner_);
        }

        return nullptr;
    }

} // namespace engine::ui::state
