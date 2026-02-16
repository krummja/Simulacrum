#ifndef UI_LAYOUT_MANAGER_HPP_
#define UI_LAYOUT_MANAGER_HPP_

namespace engine::core {
    class Context;
}

namespace engine::ui::layout {
    class UIElement;
    class UIPanel;
    class UIManager;

    class UILayoutManager {
    public:
        UILayoutManager();
        ~UILayoutManager();

        UILayoutManager(const UILayoutManager&) = delete;
        UILayoutManager& operator=(const UILayoutManager&) = delete;
        UILayoutManager(UILayoutManager&&) = delete;
        UILayoutManager& operator=(UILayoutManager&&) = delete;

    private:

    };

} // engine::ui::layout

#endif // UI_LAYOUT_MANAGER_HPP_
