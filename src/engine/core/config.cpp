#include "config.hpp"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"

namespace engine::core {

Config::Config(std::string_view filepath) {
    loadFromFile(filepath);
}

bool Config::loadFromFile(std::string_view filepath) {

    // Convert string_view to to file path (or std::string)
    auto path = std::filesystem::path(filepath);
    // ifstream does not support std::string_view construction
    std::ifstream file(path);

    if (!file.is_open()) {
        spdlog::warn(
            "Configuration file '{}' not found. Using default settings and create a default configuration file.",
            filepath
        );

        if (!saveToFile(filepath)) {
            spdlog::error(
                "Unable to create default configuration file '{}'.",
                filepath
            );
            return false;
        }

        return false;  // File does not exist; use the default value
    }

    try {
        nlohmann::json j;
        file >> j;
        spdlog::info("Successfully loaded configuration from '{}'.", filepath);
        return true;
    }

    catch (const std::exception& exc) {
        spdlog::error(
            "Error reading configuration file '{}': {}. Using default settings.",
            filepath,
            exc.what()
        );
    }

    return false;
}

bool Config::saveToFile(std::string_view filepath) {
    auto path = std::filesystem::path(filepath);
    std::ofstream file(path);

    if (!file.is_open()) {
        spdlog::error("Unable to open configuration file '{}' for writing.", filepath);
        return false;
    }

    try {
        nlohmann::ordered_json j = toJson();
        file << j.dump();
        spdlog::info("Successfully saved configuration to '{}'.", filepath);
        return true;
    }

    catch (const std::exception exc) {
        spdlog::error(
            "Error writing configuration file '{}': '{}'.",
            filepath,
            exc.what()
        );
    }

    return false;
}

void Config::fromJson(const nlohmann::json& j) {
    if (j.contains("window")) {
        const auto& window_config = j["window"];
        window_title_ = window_config.value("title", window_title_);
        window_width_ = window_config.value("width", window_width_);
        window_height_ = window_config.value("height", window_height_);
        window_resizable_ = window_config.value("resizable", window_resizable_);
    }

    if (j.contains("graphics")) {
        const auto& graphics_config = j["graphics"];
        vsync_enabled_ = graphics_config.value("vsync", vsync_enabled_);
    }

    if (j.contains("performance")) {
        const auto& perf_config = j["performance"];
        target_fps_ = perf_config.value("target_fps", target_fps_);
        if (target_fps_ < 0) {
            spdlog::warn("Target FPS cannot be native. Set to 0 (unrestricted).");
            target_fps_ = 0;
        }
    }

    if (j.contains("audio")) {
        const auto& audio_config = j["audio"];
        music_volume_ = audio_config.value("music_volume", music_volume_);
        sound_volume_ = audio_config.value("sound_volume", sound_volume_);
    }

    if (j.contains("input_mappings") && j["input_mappings"].is_object()) {
        const auto& mappings_json = j["input_mappings"];

        try {
            // Try directly converting the JSON object to a map<string, vector<string>>
            auto input_mappings = mappings_json.get<std::unordered_map<std::string, std::vector<std::string>>>();
            // If successful, move input_mappings to input_mappings_
            input_mappings_ = std::move(input_mappings);
            spdlog::trace("Input mapping successfully loaded from configuration.");
        }

        catch (const std::exception& exc) {
            spdlog::warn(
                "Configuration loading warning: An exception occurred while parsing 'input_mappings'; using defaults: {}",
                exc.what()
            );
        }
    } else {
        spdlog::trace(
            "Configuration tracing: 'input_mappings' section not found or is not an object."
        );
    }
}

nlohmann::ordered_json Config::toJson() const {
    return nlohmann::ordered_json{
        {"window", {
            {"title", window_title_},
            {"width", window_width_},
            {"height", window_height_},
            {"resizable", window_resizable_},
        }},
        {"graphics", {
            {"vsync", vsync_enabled_}
        }},
        {"performance", {
            {"target_fps", target_fps_}
        }},
        {"audio", {
            {"music_volume", music_volume_},
            {"sound_volume", sound_volume_}
        }},
        {"input_mappings", input_mappings_}
    };
}

} // namespace engine::core
