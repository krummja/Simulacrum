#ifndef UI_STATE_HPP_
#define UI_STATE_HPP_

namespace engine::core {
    class Context;
}

namespace engine::ui {
    class UIInteractive;
}

namespace engine::ui::state {

    class UIState {
        friend class UIInteractive;

    public:
        UIState(engine::ui::UIInteractive* owner): owner_(owner) {}
        virtual ~UIState() = default;

        UIState(const UIState&) = delete;
        UIState& operator=(const UIState&) = delete;
        UIState(UIState&&) = delete;
        UIState& operator=(UIState&&) = delete;

    protected:
        engine::ui::UIInteractive* owner_ = nullptr;
        virtual void enter();
        virtual std::unique_ptr<UIState> handleInput(engine::core::Context& context) = 0;

    };

} // namespace engine::ui::state


#endif // UI_STATE_HPP_