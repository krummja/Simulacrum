#include "ui_normal_state.hpp"
#include "ui_hover_state.hpp"
#include "../ui_interactive.hpp"
#include "../../input/input_manager.hpp"
#include "../../core/context.hpp"
#include <spdlog/spdlog.h>

namespace engine::ui::state {

    void UINormalState::enter() {
        owner_->setSprite("normal");
    }

    std::unique_ptr<UIState> UINormalState::handleInput(engine::core::Context& context) {
        auto& input_manager = context.getInputManager();
        auto mouse_pos = input_manager.getLogicalMousePosition();
        if (owner_->isPointInside(mouse_pos)) {
            return std::make_unique<engine::ui::state::UIHoverState>(owner_);
        }

        return nullptr;
    }

} // namespace engine::ui::state
