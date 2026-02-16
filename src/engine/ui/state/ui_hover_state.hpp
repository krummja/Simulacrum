#ifndef UI_HOVER_STATE_HPP_
#define UI_HOVER_STATE_HPP_

#include "ui_state.hpp"

namespace engine::ui::state {

    class UIHoverState final : public UIState {
        friend class engine::ui::UIInteractive;

    public:
        UIHoverState(engine::ui::UIInteractive* owner) : UIState(owner) {}
        ~UIHoverState() override = default;

    private:
        void enter() override;
        std::unique_ptr<UIState> handleInput(engine::core::Context& context) override;

    };

} // namespace engine::ui::state

#endif // UI_HOVER_STATE_HPP_