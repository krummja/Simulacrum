#ifndef UI_PRESSED_STATE_HPP_
#define UI_PRESSED_STATE_HPP_

#include "ui_state.hpp"

namespace engine::ui::state {

    class UIPressedState final : public UIState {
        friend class engine::ui::UIInteractive;

    public:
        UIPressedState(engine::ui::UIInteractive* owner) : UIState(owner) {}
        ~UIPressedState() override = default;

    private:
        void enter() override;
        std::unique_ptr<UIState> handleInput(engine::core::Context& context) override;

    };

} // namespace engine::ui::state

#endif // UI_PRESSED_STATE_HPP_