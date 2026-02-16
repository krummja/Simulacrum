#include "ui_pressed_state.hpp"
#include "ui_normal_state.hpp"
#include "ui_hover_state.hpp"
#include "../ui_interactive.hpp"
#include "../../input/input_manager.hpp"
#include "../../core/context.hpp"
#include <spdlog/spdlog.h>

namespace engine::ui::state {

    void UIPressedState::enter() {}

    std::unique_ptr<UIState> UIPressedState::handleInput(engine::core::Context& context) {
        auto& input_manager = context.getInputManager();
        auto mouse_pos = input_manager.getLogicalMousePosition();

        if (input_manager.isActionPressed("MouseLeftClick")) {
            if (!owner_->isPointInside(mouse_pos)) {
                return std::make_unique<engine::ui::state::UINormalState>(owner_);
            } else {
                owner_->clicked();
                return std::make_unique<engine::ui::state::UIHoverState>(owner_);
            }
        }

        return nullptr;
    }

} // namespace engine::ui::state
