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

        SDL_Rect toSDLRect() const { return { x, y, width, height }; }
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

        void applyPositioning(std::shared_ptr<UIComponent> component, int width, int height);
        const std::vector<std::shared_ptr<UIComponent>>& getSortedComponents() const;
        void createPanel(const std::string& id, const UIRect& bounds);

        void recordGPUVertices(GPURenderer& gpu_renderer);
        void renderGPU(GPURenderer& gpu_renderer, SDL_GPURenderPass* pass);

    private:
        bool is_shutdown_{ false };

        // Core data
        std::unordered_map<std::string, std::shared_ptr<UIComponent>> components_;

        // State tracking

        // Theme and styling
        float global_scale_{ 1.0f };

        // Settings

        // Event log state tracking

        // Window resize tracking for auto-repositioning

        // Input state

        // Performance optimization: Cached sorted components to avoid per-frame allocation + sorting
        mutable std::vector<std::shared_ptr<UIComponent>> sorted_components_cache{};
        mutable bool sorted_components_dirty_{ true };

        // Performance optimization: Value caches to avoid hash lookup when values unchanged

        // Private helper methods

        // Auto-repositioning system

        // Component-specific rendering

        // Layout helpers

        // Utility helpers

        // Deferred execution queue to prevent iterator invalidation

        std::vector<UIGPUDrawCommand> gpu_primitive_commands{};
        std::vector<UIGPUDrawCommand> gpu_text_commands{};
        std::vector<UIGPUDrawCommand> gpu_image_commands{};

        // Delete copy constructor and assignment operator
        // Copy operations
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;

        UIManager() = default;
    };

} // namespace Simulacrum
