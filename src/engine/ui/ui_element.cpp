#include "ui_element.hpp"
#include "../core/context.hpp"
#include <algorithm>
#include <utility>
#include <spdlog/spdlog.h>

namespace engine::ui {

    UIElement::UIElement(glm::vec2 position, glm::vec2 size)
        : position_(std::move(position))
        , size_(std::move(size))
    {}

    bool UIElement::handleInput(engine::core::Context& context) {
        if (!visible_) {
            return false;
        }

        // Traverse all child notes and remove elements marked for removal
        for (auto it = children_.begin(); it != children_.end();) {
            if (*it && !(*it)->isNeedRemove()) {
                if ((*it)->handleInput(context)) {
                    return true;
                }
                ++it;
            } else {
                it = children_.erase(it);
            }
        }

        return false;
    }

    void UIElement::update(float delta_time, engine::core::Context& context) {
        if (!visible_) {
            return;
        }

        for (auto it = children_.begin(); it != children_.end();) {
            if (*it && !(*it)->isNeedRemove()) {
                (*it)->update(delta_time, context);
                ++it;
            } else {
                it = children_.erase(it);
            }
        }
    }

    void UIElement::render(engine::core::Context& context) {
        if (!visible_) {
            return;
        }

        for (const auto& child : children_) {
            if (child) {
                child->render(context);
            }
        }
    }

    void UIElement::addChild(std::unique_ptr<UIElement> child) {
        if (child) {
            child->setParent(this);
            children_.push_back(std::move(child));
        }
    }

    std::unique_ptr<UIElement> UIElement::removeChild(UIElement* child_ptr) {
        auto it = std::find_if(
            children_.begin(),
            children_.end(),
            [child_ptr](const std::unique_ptr<UIElement>& p) {
                return p.get() == child_ptr;
            }
        );

        if (it != children_.end()) {
            std::unique_ptr<UIElement> removed_child = std::move(*it);
            children_.erase(it);
            removed_child->setParent(nullptr);
            return removed_child;
        }

        return nullptr;
    }

    void UIElement::removeAllChildren() {
        for (auto& child : children_) {
            child->setParent(nullptr);
        }

        children_.clear();
    }

    glm::vec2 UIElement::getScreenPosition() const {
        if (parent_) {
            return parent_->getScreenPosition() + position_;
        }

        return position_;
    }

    engine::utils::Rect UIElement::getBounds() const {
        auto abs_pos = getScreenPosition();
        // return engine::utils::Rect(abs_pos, size_);
        return engine::utils::Rect{abs_pos, size_};
    }

    bool UIElement::isPointInside(const glm::vec2& point) const {
        auto bounds = getBounds();
        return (point.x >= bounds.position.x && point.x < (bounds.position.x + bounds.size.x) &&
                point.y >= bounds.position.y && point.y < (bounds.position.y + bounds.size.y));
    }

}
