#ifndef UI_NORMAL_STATE_HPP_
#define UI_NORMAL_STATE_HPP_

#include "ui_state.hpp"

namespace engine::ui::state {

    class UINormalState final : public UIState {
        friend class engine::ui::UIInteractive;

    public:
        UINormalState(engine::ui::UIInteractive* owner) : UIState(owner) {}
        ~UINormalState() override = default;

    private:
        void enter() override;
        std::unique_ptr<UIState> handleInput(engine::core::Context& context) override;

    };

} // namespace engine::ui::state

#endif // UI_NORMAL_STATE_HPP_