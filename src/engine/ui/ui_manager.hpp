#ifndef UI_MANAGER_HPP_
#define UI_MANAGER_HPP_

#include <memory>
#include <vector>
#include <glm/vec2.hpp>

namespace engine::core {
    class Context;
}

namespace engine::ui {
    // class UIElement;
    // class UIPanel;

    class UIManager final {
    public:
        UIManager();
        ~UIManager();

        [[nodiscard]] bool init(const glm::vec2& window_size);
        // void addElement(std::unique_ptr<UIElement> element);
        // UIPanel* getRootElement() const;
        // void clearElements();

        // bool handleInput(engine::core::Context&);
        // void update(float delta_time, engine::core::Context&);
        // void render(engine::core::Context&);

        // UIManager(const UIManager&) = delete;
        // UIManager& operator=(const UIManager&) = delete;
        // UIManager(UIManager&&) = delete;
        // UIManager& operator=(UIManager&&) = delete;

    private:
        // std::unique_ptr<UIPanel> root_element_;

    };
} // namespace engine::ui

#endif // UI_MANAGER_HPP_