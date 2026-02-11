#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <nlohmann/json_fwd.hpp>  // nlohmann_json forward declaration provided

namespace engine::core {

/**
 * @brief Manage application configuration settings
 * Provides default values for configuration items and support loading/saving
 * configurations from JSON files. Default values will be used if loading fails or if the
 * file does not exist.
 */
class Config final {

public:
    // --- Default configuration values --- for ease of expansion, all are set to public
    // Window settings
    std::string window_title_ = "Simulacrum";
    int window_width_ = 1280;
    int window_height_ = 720;
    bool window_resizable_ = true;

    // Graphics settings
    bool vsync_enabled_ = true;

    // Performance settings
    int target_fps_ = 144;

    // Audio settings
    float music_volume_ = 0.5f;
    float sound_volume_ = 0.5f;

    // Store the mapping of action names to the SDL scancode name list
    std::unordered_map<std::string, std::vector<std::string>> input_mappings_ = {
        {"move_left", {"A", "Left"}},
        {"move_right", {"D", "Right"}},
        {"move_up", {"W", "Up"}},
        {"move_down", {"S", "Down"}},
    };

    explicit Config(std::string_view filepath);

    // Delete copy and move semantics
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    bool loadFromFile(std::string_view filepath);
    [[nodiscard]] bool saveToFile(std::string_view filepath);

private:
    void fromJson(const nlohmann::json& j);
    nlohmann::ordered_json toJson() const;

};

} // namespace engine::core
