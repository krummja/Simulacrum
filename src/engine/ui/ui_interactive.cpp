#include "ui_interactive.hpp"
#include "state/ui_state.hpp"
#include "../core/context.hpp"
#include "../render/renderer.hpp"
#include "../resource/resource_manager.hpp"
#include <spdlog/spdlog.h>

namespace engine::ui {

    UIInteractive::UIInteractive(
        engine::core::Context& context,
        glm::vec2 position,
        glm::vec2 size
    )
        : UIElement(std::move(position), std::move(size))
        , context_(context)
    {}

    UIInteractive::~UIInteractive() = default;

    void UIInteractive::setState(std::unique_ptr<engine::ui::state::UIState> state) {
        if (!state) {
            return;
        }

        state_ = std::move(state);
        state_->enter();
    }

    void UIInteractive::addSprite(std::string_view name, std::unique_ptr<engine::render::Sprite> sprite) {
        if (size_.x == 0.0f && size_.y == 0.0f) {
            size_ = context_.getResourceManager().getTextureSize(sprite->getTextureId());
        }

        sprites_[std::string(name)] = std::move(sprite);
    }

    void UIInteractive::setSprite(std::string_view name) {
        if (sprites_.find(std::string(name)) != sprites_.end()) {
            current_sprite_ = sprites_[std::string(name)].get();
        }
    }

    void UIInteractive::addSound(std::string_view name, std::string_view path) {
        sounds_[std::string(name)] = path;
    }

    void UIInteractive::playSound(std::string_view name) {
        if (sounds_.find(std::string(name)) != sounds_.end()) {
            // TODO
        }
    }

    bool UIInteractive::handleInput(engine::core::Context& context) {
        if (UIElement::handleInput(context)) {
            return true;
        }

        if (state_ && interactive_) {
            if (auto next_state = state_->handleInput(context); next_state) {
                setState(std::move(next_state));
                return true;
            }
        }

        return false;
    }

    void UIInteractive::render(engine::core::Context& context) {
        if (!visible_) return;
        context.getRenderer().drawUISprite(*current_sprite_, getScreenPosition(), size_);
        UIElement::render(context);
    }

} // namespace engine::ui
