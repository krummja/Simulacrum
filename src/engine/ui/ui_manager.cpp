#include "ui_manager.hpp"
#include "ui_panel.hpp"
#include "ui_element.hpp"
#include <spdlog/spdlog.h>

namespace engine::ui {

    UIManager::UIManager() {
        root_element_ = std::make_unique<UIPanel>(glm::vec2{0.0f, 0.0f}, glm::vec2{0.0f, 0.0f});
    }

    UIManager::~UIManager() = default;

    bool UIManager::init(const glm::vec2& window_size) {
        root_element_->setSize(window_size);
        spdlog::debug("Initialized UIManager (Root element: w: {}, h: {})", window_size.x, window_size.y);
        return true;
    }

    void UIManager::addElement(std::unique_ptr<UIElement> element) {
        if (root_element_) {
            root_element_->addChild(std::move(element));
        }
    }

    void UIManager::clearElements() {
        if (root_element_) {
            root_element_->removeAllChildren();
        }
    }

    bool UIManager::handleInput(engine::core::Context& context) {
        if (root_element_ && root_element_->isVisible()) {
            if (root_element_->handleInput(context)) {
                return true;
            }
        }

        return false;
    }

    void UIManager::update(float delta_time, engine::core::Context& context) {
        if (root_element_ && root_element_->isVisible()) {
            root_element_->update(delta_time, context);
        }
    }

    void UIManager::render(engine::core::Context& context) {
        if (root_element_ && root_element_->isVisible()) {
            root_element_->render(context);
        }
    }

    UIPanel* UIManager::getRootElement() const {
        return root_element_.get();
    }

} // namespace engine::ui
