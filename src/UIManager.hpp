#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

#include "UIConstants.hpp"

struct SDL_GPURenderPass;

using Vector2D = glm::vec2;

namespace Simulacrum
{
    class GPURenderer;

    struct UIGPUDrawCommand
    {
        enum class Type { Rect, Text, Image };
        Type type{ Type::Rect };
        SDL_GPUTexture* texture{ nullptr };
        uint32_t vertex_offset{ 0 };
        uint32_t vertex_count{ 0 };
    };

    // GPU command buffer capacities (avoids per-frame reallocations)
    constexpr size_t GPU_PRIMITIVE_COMMAND_CAPACITY = 256;
    constexpr size_t GPU_TEXT_COMMAND_CAPACITY = 64;
    constexpr size_t GPU_IMAGE_COMMAND_CAPACITY = 32;

    // GPU vertex safety limits
    constexpr uint32_t GPU_PRIMITIVE_VERTEX_LIMIT = 10000;
    constexpr uint32_t GPU_UI_VERTEX_LIMIT = 4000;

    enum class UIComponentType
    {
        BUTTON,
        BUTTON_DANGER,
        BUTTON_SUCCESS,
        BUTTON_WARNING,
        LABEL,
        TITLE,
        PANEL,
        PROGRESS_BAR,
        INPUT_FIELD,
        IMAGE,
        SLIDER,
        CHECKBOX,
        LIST,
        TOOLTIP,
        EVENT_LOG,
        DIALOG
    };

    enum class UILayoutType { ABSOLUTE_POS, FLOW, GRID, STACK, ANCHOR };

    enum class UIPositionMode
    {
        ABSOLUTE_,
        CENTERED_H,
        CENTERED_V,
        CENTERED_BOTH,
        TOP_ALIGNED,
        TOP_RIGHT,
        BOTTOM_ALIGNED,
        BOTTOM_CENTERED,
        BOTTOM_RIGHT,
        LEFT_ALIGNED,
        RIGHT_ALIGNED
    };

    struct UIPositioning
    {
        UIPositionMode mode{ UIPositionMode::ABSOLUTE_ };

        /// @brief Horizontal offset from positioning anchor
        int offset_x{ 0 };

        /// @brief Vertical offset from positioning anchor
        int offset_y{ 0 };

        /// @brief Fixed width (0 = use current width)
        int fixed_width{ 0 };

        /// @brief Fixed height (0 = use current height)
        int fixed_height{ 0 };

        /// @brief Width as fraction of window (e.g. 0.57 = 57%), takes precedence over fixed width
        float width_percentage{ 0.0f };

        /// @brief Height as fraction of window, takes precedence over fixed height
        float height_percentage{ 0.0f };
    };

    enum class UIState { NORMAL, HOVERED, PRESSED, DISABLED, FOCUSED };

    enum class UIAlignment
    {
        LEFT,
        CENTER,
        RIGHT,
        TOP,
        BOTTOM,
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER_CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };

    struct UIRect
    {
        int x{ 0 };
        int y{ 0 };
        int width{ 0 };
        int height{ 0 };

        UIRect() = default;
        UIRect(int x_, int y_, int w_, int h_)
            : x(x_), y(y_), width(w_), height(h_)
        {}

        bool contains(int px, int py) const
        {
            return px >= x && px < x + width && py >= y && py < y + height;
        }

        SDL_Rect toSDLRect() const
        {
            return { x, y, width, height };
        }
    };

    struct UIStyle
    {
        SDL_Color background_color{ 255, 255, 255, 255 };
        SDL_Color border_color{ 100, 100, 100, 255 };
        SDL_Color text_color{ 255, 255, 255, 255 };
        SDL_Color hover_color{ 70, 70, 70, 255 };
        SDL_Color pressed_color{ 30, 30, 30, 255 };
        SDL_Color disabled_color{ 80, 80, 80, 128 };

        SDL_Color text_background_color{ 0, 0, 0, 128 };

        bool use_text_background{ false };

        int text_background_padding{ UIConstants::DEFAULT_TEXT_BG_PADDING };
        int border_width{ UIConstants::BORDER_WIDTH_NORMAL };
        int padding{ UIConstants::DEFAULT_COMPONENT_PADDING };
        int margin{ UIConstants::DEFAULT_MARGIN };
        int list_item_height{ UIConstants::DEFAULT_LIST_ITEM_HEIGHT };

        std::string font_id{ UIConstants::FONT_UI };
        int font_size{ UIConstants::DEFAULT_FONT_SIZE };

        UIAlignment text_align{ UIAlignment::CENTER_CENTER };
    };

    struct UIComponent
    {
        std::string id{};
        UIComponentType type{};
        UIRect bounds{};
        UIState state{ UIState::NORMAL };
        UIStyle style{};
        bool visible{ true };
        bool enabled{ true };
        int z_order{ 0 };

        // Auto-repositioning properties
        UIPositioning positioning{};

        // Auto-sizing properties
        bool auto_size_{ true };
        UIRect min_bounds{ 0, 0, UIConstants::MIN_COMPONENT_WIDTH, UIConstants::MIN_COMPONENT_HEIGHT };
        UIRect max_bounds{ 0, 0, UIConstants::MAX_COMPONENT_WIDTH, UIConstants::MAX_COMPONENT_HEIGHT };
        int content_padding{ UIConstants::DEFAULT_CONTENT_PADDING };
        bool auto_width{ true };
        bool auto_height{ true };
        bool size_to_content{ true };

        // Component-specific data
        std::string text{};
        std::function<std::string()> text_binding{};
        bool binding_dirty{ true };
        std::string texture_id{};
        float value{ 0.0f };
        float min_value{ 0.0f };
        float max_value{ 1.0f };
        bool checked{ false };
        std::vector<std::string> list_items{};
        std::vector<std::shared_ptr<SDL_Texture>> list_item_textures{};
        bool list_items_dirty{ true };
        std::function<void(std::vector<std::string>&, std::vector<std::pair<std::string, int>>&)> list_binding{};
        mutable std::vector<std::string> list_binding_buffer{};
        mutable std::vector<std::pair<std::string, int>> list_sort_buffer{};
        int selected_index{ -1 };
        std::string placeholder{};
        int max_length{ UIConstants::DEFAULT_INPUT_MAX_LENGTH };

        // Callbacks
        std::function<void()> on_click{};
        std::function<void(float)> on_value_changed{};
        std::function<void(const std::string&)> on_text_changed{};
        std::function<void()> on_hover{};
        std::function<void()> on_focus{};
        std::function<void()> on_content_changed{};

        virtual ~UIComponent() = default;
    };

    struct UILayout
    {
        std::string id{};
        UILayoutType type{ UILayoutType::ABSOLUTE_POS };
        UIRect bounds{};
        std::vector<std::string> child_components{};

        int spacing{ UIConstants::DEFAULT_LAYOUT_SPACING };
        int columns{ 1 };
        int rows{ 1 };
        UIAlignment alignment{ UIAlignment::TOP_LEFT };
        bool auto_size{ false };
    };

    struct UITheme
    {
        std::string name{ "default" };
        std::unordered_map<UIComponentType, UIStyle> component_styles{};

        UIStyle getStyle(UIComponentType type) const
        {
            auto it = component_styles.find(type);
            return (it != component_styles.end()) ? it->second : UIStyle{};
        }
    };

    struct UIAnimation
    {
        std::string component_id{};
        float duration{ 0.0f };
        float elapsed{ 0.0f };
        bool active{ false };

        UIRect start_bounds{};
        UIRect target_bounds{};
        SDL_Color start_color{};
        SDL_Color end_color{};

        std::function<void()> on_complete{};
    };

    struct EventLogState
    {
        float timer{ 0.0f };
        int message_index{ 0 };
        float update_interval{ 2.0f };
        bool auto_update{ false };
    };

    class UIManager
    {
    public:
        ~UIManager()
        {
            if (!is_shutdown_)
            {
                clean();
            }
        }

        static UIManager& Instance()
        {
            static UIManager instance;
            return instance;
        }

        bool init();
        void update(float delta_time);
        void clean();
        bool isShutdown() const { return is_shutdown_; }

        // Component builders
        void createPanel(const std::string& id, const UIRect& bounds);
        void createProgressBar(const std::string& id, const UIRect& bounds, float min_val = 0.0f, float max_val = 1.0f);
        void createInputField(const std::string& id, const UIRect& bounds, const std::string& placeholder = "");
        void createImage(const std::string& id, const UIRect& bounds, const std::string& texture_id = "");
        void createSlider(const std::string& id, const UIRect& bounds, float min_val = 0.0f, float max_val = 1.0f);
        void createCheckbox(const std::string& id, const UIRect& bounds, const std::string& text = "");
        void createList(const std::string& id, const UIRect& bounds);
        void createTooltip(const std::string& id, const std::string& text = "");
        void createEventLog(const std::string& id, const UIRect& bounds, int max_entries = UIConstants::DEFAULT_EVENT_LOG_MAX_ENTRIES);
        void createDialog(const std::string& id, const UIRect& bounds);
        void createModal(const std::string& dialog_id, const UIRect& bounds, const std::string& theme, int window_width, int window_height);
        void createLabel(const std::string& id, const UIRect& bounds, const std::string& text = "");

        void refreshAllComponentThemes() const;

        void removeComponent(const std::string& id);
        void clearAllComponents();
        bool hasComponent(const std::string& id) const;
        void setComponentVisible(const std::string& id, bool visible);
        void setComponentEnabled(const std::string& id, bool enabled);
        void setComponentBounds(const std::string& id, const UIRect& bounds);
        void setComponentZOrder(const std::string& id, int z_order);
        void setComponentPositioning(const std::string& id, const UIPositioning& positioning);

        void setText(const std::string& id, const std::string& text);
        void setTexture(const std::string& id, const std::string& texture_id);
        void setValue(const std::string& id, float value);
        void setChecked(const std::string& id, bool checked);
        void setStyle(const std::string& id, const UIStyle& style);

        void bindText(const std::string& id, std::function<std::string()> binding);
        void bindList(const std::string& id, std::function<void(std::vector<std::string>&, std::vector<std::pair<std::string, int>>&)> binding);
        void markBindingDirty(const std::string& id);
        void markAllBindingsDirty();

        std::string getText(const std::string& id) const;
        float getValue(const std::string& id) const;
        UIRect getBounds(const std::string& id) const;
        UIState getComponentState(const std::string& id) const;

        bool isButtonClicked(const std::string& id) const;
        bool isButtonPressed(const std::string& id) const;
        bool isButtonHovered(const std::string& id) const;
        bool isComponentFocused(const std::string& id) const;

        void createLayout(const std::string& id, UILayoutType type, const UIRect& bounds);
        void addComponentToLayout(const std::string& layout_id, const std::string& component_id);
        void removeComponentFromLayout(const std::string& layout_id, const std::string& component_id);
        void updateLayout(const std::string& layout_id);
        void setLayoutSpacing(const std::string& layout_id, int spacing);
        void setLayoutColumns(const std::string& layout_id, int columns);
        void setLayoutAlignment(const std::string& layout_id, UIAlignment alignment);

        void updateProgressBar(const std::string& id, float value);
        void setProgressBarRange(const std::string& id, float min_val, float max_val);

        void addListItem(const std::string& id, const std::string& item);
        void removeListItem(const std::string& id, int index);
        void clearList(const std::string& id);
        int getSelectedListItem(const std::string& id) const;
        void setSelectedListItem(const std::string& id, int index);

        void setListMaxItems(const std::string& id, int max_items);
        void addListItemWithAutoScroll(const std::string& id, const std::string& item);
        void clearListItems(const std::string& id);

        void addEventLogEntry(const std::string id, const std::string& entry);
        void clearEventLog(const std::string& id);
        void setEventLogMaxEntries(const std::string& id, int max_entries);
        void enableEventLogAutoUpdate(const std::string& id, float interval = UIConstants::DEFAULT_EVENT_LOG_UPDATE_INTERVAL);
        void disableEventLogAutoUpdate(const std::string& id);

        void setTitleAlignment(const std::string& id, UIAlignment alignment);
        void centerTitleInContainer(const std::string& id, int container_x, int container_width);

        void setLabelAlignment(const std::string& id, UIAlignment alignment);

        void setInputFieldPlaceholder(const std::string& id, const std::string& placeholder);
        void setInputFieldMaxLength(const std::string& id, int max_length);
        bool isInputFieldFocused(const std::string& id) const;

        void animateMove(const std::string& id, const UIRect& target_bounds, float duration, std::function<void()> on_complete = nullptr);
        void animateColor(const std::string& id, const SDL_Color& target_color, float duration, std::function<void()> on_complete = nullptr);
        void stopAnimation(const std::string& id);
        bool isAnimating(const std::string& id) const;

        void loadTheme(const UITheme& theme);
        void setDefaultTheme();
        void setLightTheme();
        void setDarkTheme();
        void setThemeMode(const std::string& mode);
        void applyThemeToComponent(const std::string& id, UIComponentType type);
        void setGlobalStyle(const UIStyle& style);

        void createOverlay(int window_width, int window_height);
        void createOverlay();
        void removeOverlay();

        void enableTextBackground(const std::string& id, bool enable = true);
        void setTextBackgroundColor(const std::string& id, SDL_Color color);
        void setTextBackgroundPadding(const std::string& id, int padding);

        void removeComponentWithPrefix(const std::string& prefix);
        void resetToDefaultTheme();

        void cleanupForStateTransition();
        void prepareForStateTransition();

        void calculateOptimalSize(const std::string& id);
        void calculateOptimalSize(std::shared_ptr<UIComponent> component);
        bool measureComponentContent(const std::shared_ptr<UIComponent>& component, int* width, int* height);

        void invalidateLayout(const std::string& layout_id);
        void recalculateLayout(const std::string& layout_id);
        void enableAutoSizing(const std::string& id, bool enable = true);
        void setAutoSizingConstraints(const std::string& id, const UIRect& min_bounds, const UIRect& max_bounds);

        int getLogicalWidth() const;
        int getLogicalHeight() const;
        void createTitleAtTop(const std::string& id, const std::string& text, int height = UIConstants::DEFAULT_TITLE_HEIGHT);
        void createButtonAtBottom(const std::string& id, const std::string& text, int width = UIConstants::DEFAULT_BUTTON_WIDTH, int height = UIConstants::DEFAULT_BUTTON_HEIGHT);
        void createCenteredDialog(const std::string& id, int width, int height, const std::string& theme = "dark");
        void createCenteredButton(const std::string& id, int offset_y, int width, int height, const std::string& text);

        void createPanelAtBottomRight(const std::string& id, int width, int height, int offset_x = UIConstants::BOTTOM_RIGHT_OFFSET_X, int offset_y = UIConstants::BOTTOM_RIGHT_OFFSET_Y);

        void createLabelAtBottomRight(const std::string& id, const std::string& text, int width, int height, int offset_x = UIConstants::BOTTOM_RIGHT_OFFSET_X, int offset_y = UIConstants::BOTTOM_RIGHT_OFFSET_Y);

        void setGlobalFont(const std::string& font_id);
        void setGlobalScale(float scale);
        float getGlobalScale() const { return global_scale_; }

        inline UIRect scaleRect(const UIRect& bounds) const
        {
            return {
                static_cast<int>(bounds.x * global_scale_),
                static_cast<int>(bounds.y * global_scale_),
                static_cast<int>(bounds.width * global_scale_),
                static_cast<int>(bounds.height * global_scale_)
            };
        }

        float calculateOptimalScale(int width, int height) const;
        void enableTooltips(bool enable) { tooltips_enabled_ = enable; }
        void setTooltipDelay(float delay) { tooltip_delay_ = delay; }

        void setDebugMode(bool enable) { debug_mode_ = enable; }
        void drawDebugBounds(bool enable) { draw_debug_bounds_ = enable; }
        bool isClickOnUI(const Vector2D& screen_pos) const;

        void recordGPUVertices(GPURenderer& gpu_renderer);
        void renderGPU(GPURenderer& gpu_renderer, SDL_GPURenderPass* pass);

        void onWindowResize(int logical_width, int logical_height);

    private:

        // Core data
        std::unordered_map<std::string, std::shared_ptr<UIComponent>> components_;
        std::unordered_map<std::string, std::shared_ptr<UILayout>> layouts_;
        std::vector<std::shared_ptr<UIAnimation>> animations_;

        // State tracking
        std::vector<std::string> clicked_buttons_{};
        std::vector<std::string> hovered_components_{};
        std::string focused_component_{};
        std::string hovered_tooltip_{};
        float tooltip_timer{ 0.0f };

        // Theme and styling
        UITheme current_theme_{};
        UIStyle global_style_{};
        std::string global_font_id_{ UIConstants::FONT_DEFAULT };
        std::string title_font_id_{ UIConstants::FONT_TITLE };
        std::string ui_font_id_{ UIConstants::FONT_UI };
        float global_scale_{ 1.0f };
        std::string current_theme_mode_{ "light" };

        // Settings
        bool tooltips_enabled_{ true };
        float tooltip_delay_{ 1.0f };
        bool debug_mode_{ false };
        bool draw_debug_bounds_{ false };

        // Event log state tracking
        std::unordered_map<std::string, EventLogState> event_log_states{};

        bool is_shutdown_{ false };

        // Window resize tracking for auto-repositioning
        int current_logical_width_{ 0 };
        int current_logical_height_{ 0 };

        // Input state
        Vector2D last_mouse_position_{};
        bool mouse_pressed_{ false };
        bool mouse_released_{ false };

        // Performance optimization: Cached sorted components to avoid per-frame allocation + sorting
        mutable std::vector<std::shared_ptr<UIComponent>> sorted_components_cache_{};
        mutable bool sorted_components_dirty_{ true };

        // Performance optimization: Value caches to avoid hash lookup when values unchanged
        std::unordered_map<std::string, float> value_cache_{};
        std::unordered_map<std::string, std::string> text_cache_{};

        // Private helper methods
        std::shared_ptr<UIComponent> getComponent(const std::string& id);
        std::shared_ptr<const UIComponent> getComponent(const std::string& id) const;
        std::shared_ptr<UILayout> getLayout(const std::string& id);

        // Auto-repositioning system
        void applyPositioning(std::shared_ptr<UIComponent> component, int width, int height);

        void repositionAllComponents(int width, int height);
        void handleInput();
        void updateAnimations(float delta_time);
        void updateTooltips(float delta_time);
        void updateEventLogs(float delta_time);

        // Performance: Return const reference to avoid vector copy every frame
        const std::vector<std::shared_ptr<UIComponent>>& getSortedComponents() const;

        // Performance optimization helper
        void invalidateComponentCache();

        // Layout helpers
        void applyAbsoluteLayout(const std::shared_ptr<UILayout>& layout);
        void applyFlowLayout(const std::shared_ptr<UILayout>& layout);
        void applyGridLayout(const std::shared_ptr<UILayout>& layout);
        void applyStackLayout(const std::shared_ptr<UILayout>& layout);
        void applyAnchorLayout(const std::shared_ptr<UILayout>& layout);

        // Utility helpers
        SDL_Color interpolateColor(const SDL_Color& start, const SDL_Color& end, float t);
        UIRect interpolateRect(const UIRect& start, const UIRect& end, float t);

        void executeDeferredCallbacks();

        // Deferred execution queue to prevent iterator invalidation
        std::vector<std::function<void()>> deferred_callbacks_{};

        size_t active_binding_count_{ 0 };

        std::vector<UIGPUDrawCommand> gpu_primitive_commands_{};
        std::vector<UIGPUDrawCommand> gpu_text_commands_{};
        std::vector<UIGPUDrawCommand> gpu_image_commands_{};

        // Delete copy constructor and assignment operator
        // Copy operations
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;

        UIManager() = default;
    };

} // namespace Simulacrum
