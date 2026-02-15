#ifndef UI_INTERACTIVE_HPP_
#define UI_INTERACTIVE_HPP_

#include "ui_element.hpp"
#include "state/ui_state.hpp"
#include "../render/sprite.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace engine::core {
    class Context;
}

namespace engine::ui {

    class UIInteractive : public UIElement {
    public:
        UIInteractive(
            engine::core::Context& context,
            glm::vec2 position = {0.0f, 0.0f},
            glm::vec2 size = {0.0f, 0.0f}
        );

        ~UIInteractive() override;

        virtual void clicked() {}

        void addSprite(std::string_view name, std::unique_ptr<engine::render::Sprite> sprite);
        void setSprite(std::string_view name);
        void addSound(std::string_view name, std::string_view path);
        void playSound(std::string_view name);

        void setState(std::unique_ptr<engine::ui::state::UIState> state);
        engine::ui::state::UIState* getState() const { return state_.get(); }

        void setInteractive(bool interactive) { interactive_ = interactive; }
        bool isInteractive() const { return interactive_; }

        bool handleInput(engine::core::Context& context) override;
        void render(engine::core::Context& context) override;

    protected:
        engine::core::Context& context_;
        std::unique_ptr<engine::ui::state::UIState> state_;
        std::unordered_map<std::string, std::unique_ptr<engine::render::Sprite>> sprites_;
        std::unordered_map<std::string, std::string> sounds_;
        engine::render::Sprite* current_sprite_ = nullptr;
        bool interactive_ = true;
    };

} // namespace engine::ui

#endif // UI_INTERACTIVE_HPP_