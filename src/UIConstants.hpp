#pragma once

#include <string_view>

namespace Simulacrum::UIConstants
{

    constexpr std::string_view FONT_UI = "fonts_UI_Arial";
    constexpr std::string_view FONT_TITLE = "fonts_title_Arial";
    constexpr std::string_view FONT_TOOLTIP = "fonts_tooltip_Arial";
    constexpr std::string_view FONT_DEFAULT = "fonts_UI_Arial";

    // UI Baseline Resolution (for scaling calculations)
    // This is the reference resolution used for UI design - all UI elements are defined
    // in this coordinate space and then scaled to the actual window resolution
    constexpr int BASELINE_WIDTH = 1920;
    constexpr int BASELINE_HEIGHT = 1080;
    constexpr float BASELINE_WIDTH_F = 1920.0f;
    constexpr float BASELINE_HEIGHT_F = 1080.0f;

    // UI Component Spacing Constants (in baseline pixels)
    // These values are scaled by global_scale_ at runtime for resolution-aware spacing
    constexpr int CHECKBOX_SIZE = 24;
    constexpr int TOOLTIP_PADDING_WIDTH = 16;
    constexpr int TOOLTOP_PADDING_HEIGHT = 8;
    constexpr int TOOLTIP_MOUSE_OFFSET = 10;
    constexpr int INPUT_CURSOR_SPACE = 20;
    constexpr int LIST_ITEM_PADDING = 8;
    constexpr int SCROLLBAR_WIDTH = 20;

    // Z-Order Layering Constants
    // Controls the render order of UI components (lower values render first/behind)
    constexpr int ZORDER_OVERLAY = -10;
    constexpr int ZORDER_PANEL = 0;
    constexpr int ZORDER_IMAGE = 1;
    constexpr int ZORDER_DIALOG = 2;
    constexpr int ZORDER_PROGRESS_BAR = 5;
    constexpr int ZORDER_EVENT_LOG = 6;
    constexpr int ZORDER_lIST = 8;
    constexpr int ZORDER_BUTTON = 10;
    constexpr int ZORDER_SLIDER = 12;
    constexpr int ZORDER_CHECKBOX = 13;
    constexpr int ZORDER_INPUT_FIELD = 15;
    constexpr int ZORDER_LABEL = 20;
    constexpr int ZORDER_TITLE = 25;
    constexpr int ZORDER_TOOLTIP = 1000;

    // Border Width Constants
    constexpr int BORDER_WIDTH_NONE = 0;
    constexpr int BORDER_WIDTH_NORMAL = 1;
    constexpr int BORDER_WIDTH_DIALOG = 2;
    constexpr int DEBUG_BORDER_WIDTH = 1;

    // Font Size Constants
    constexpr int DEFAULT_FONT_SIZE = 16;
    constexpr int TITLE_FONT_SIZE = 24;

    // Component Default Size Constants
    constexpr int MIN_COMPONENT_WIDTH = 32;
    constexpr int MIN_COMPONENT_HEIGHT = 16;
    constexpr int MAX_COMPONENT_WIDTH = 800;
    constexpr int MAX_COMPONENT_HEIGHT = 600;
    constexpr int DEFAULT_INPUT_MAX_LENGTH = 256;

    // Dialog/Modal Size Constants (baseline pixels, auto-scaled by UIManager)
    constexpr int DEFAULT_DIALOG_WIDTH = 400;
    constexpr int DEFAULT_DIALOG_HEIGHT = 200;

    // Padding and Spacing Defaults
    constexpr int DEFAULT_COMPONENT_PADDING = 8;
    constexpr int DEFAULT_CONTENT_PADDING = 8;
    constexpr int DEFAULT_MARGIN = 4;
    constexpr int DEFAULT_LAYOUT_SPACING = 4;
    constexpr int DEFAULT_TEXT_BG_PADDING = 4;
    constexpr int LABEL_TEXT_BG_PADDING = 6;
    constexpr int TITLE_TEXT_BG_PADDING = 8;

    // List Component Constants
    constexpr int DEFAULT_LIST_ITEM_HEIGHT = 32;
    constexpr int FALLBACK_LIST_ITEM_HEIGHT = 29;
    constexpr int MIN_LIST_WIDTH = 150;
    constexpr int DEFAULT_LIST_WDTH = 200;
    constexpr int DEFAULT_LIST_VISIBLE_ITEMS = 3;
    constexpr int MAX_LIST_TEXTURE_CACHE = 1000;

    // Slider Component Constants
    constexpr int SLIDER_TRACK_HEIGHT = 4;
    constexpr int SLIDER_TRACK_OFFSET = 2;
    constexpr int SLIDER_HANDLE_WIDTH = 20;

    // Tooltip Constants
    constexpr int TOOLTIP_FALLBACK_WIDTH = 200;
    constexpr int TOOLTIP_FALLBACK_HEIGHT = 32;

    // Event Log Constants
    constexpr int DEFAULT_EVENT_LOG_MMAX_ENTRIES = 5;
    constexpr float DEFAULT_EVENT_LOG_UPDATE_INTERVAL = 2.0f;
    constexpr float EVENT_LONG_WIDTH_PERCENT = 0.30F;

    // Time Status Bar Constants
    constexpr int TIME_STATUS_WIDTH = 280;
    constexpr int TIME_STATUS_HEIGHT = 32;
    constexpr int TIME_STATUS_TOP_OFFSET = 10;
    constexpr int TIME_STATUS_RIGHT_OFFSET = 10;

    // Full-Width Status Bar Constants
    constexpr int STATUS_BAR_HEIGHT = 40;
    constexpr int STATUS_BAR_LABEL_PADDING = 12;

    // Timing and Animation Constants
    constexpr float DEFAULT_TOOLTIP_DELAY = 1.0f;
    constexpr float MAX_UI_SCALE = 3.0f;

    // Positioning Constants
    constexpr int TITLE_TOP_OFFSET = 10;
    constexpr int BUTTON_BOTTOM_OFFSET = 20;
    constexpr int BOTTOM_RIGHT_OFFSET_X = 10;
    constexpr int BOTTOM_RIGHT_OFFSET_Y = 10;
    constexpr int DEFAULT_BUTTON_WIDTH = 120;
    constexpr int DEFAULT_BUTTON_HEIGHT = 40;

    // Info/Status Label Sizing Constants (baseline pixels, auto-scaled by UIManager)
    constexpr int INFO_lABEL_HEIGHT = 36;
    constexpr int INFO_LABEL_HEIGHT_STANDARD = 28;
    constexpr int INFO_LABEL_HEIGHT_COMPACT = 24;

    // Info Label Positioning Constants (baseline pixels)
    constexpr int INFO_FIRST_LINE_Y = 62;
    constexpr int INFO_LINE_SPACING = 8;
    constexpr int INFO_STATUS_SPACING = 4;
    constexpr int INFO_LABEL_MARGIN_X = 10;

    // Form/Settings Layout Constants (reusable for any form-line interface)
    constexpr int CONTENT_START_Y_AFTER_TABS = 160;
    constexpr int FORM_ROW_HEIGHT = 60;
    constexpr int FORM_LABEL_WIDTH = 250;
    constexpr int FORM_CONTROL_WIDTH = 300;
    constexpr int FORM_LABEL_CONTROL_GAP = 20;
    constexpr int DEFAULT_SLIDER_HEIGHT = 30;
    constexpr int BOTTOM_BUTTON_MARGIN = 80;

    // Text Mesurement Estimates
    constexpr int CHAR_WIDTH_ESTIMATE = 12;
    constexpr int INPUT_CURSOR_CHAR_WIDTH = 8;

    // Performance/Memory Constants
    constexpr int DEFAULT_COMPONENT_BATCH_SIZE = 32;
    constexpr int MAX_COMPONENT_BATCH_SIZE = 64;

    // Debug Profiler Overlay Constants (Debug builds only)
    constexpr int PROFILER_OVERLAY_WIDTH = 300;
    constexpr int PROFILER_OVERLAY_HEIGHT = 172;
    constexpr int PROFILER_OVERLAY_MARGIN = 10;
    constexpr int PROFILER_LINE_HEIGHT = 22;
    constexpr int PROFILER_LABEL_COUNT = 7;
    constexpr int PROFILER_ZORDER_PANEL = 9000;
    constexpr int PROFILER_ZORDER_LABEL = 9001;

} // namespace Simulacrum::UIConstants
