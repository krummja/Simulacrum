#pragma once

#include "macros.hpp"

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Simulacrum
{

  struct GraphicsSettings
  {
    bool vsync;
    int resolution_height;
    int resolution_width;
    bool fullscreen;
    int buffer_count;
    bool show_fps;
    int fps_limit;
  };

  class SettingsManager
  {
  public:
    ~SettingsManager() = default;

    /// @brief Gets the singleton instance of the SettingsManager
    /// @return Reference to the SettingsManager singleton instance
    static SettingsManager& Instance()
    {
      static SettingsManager instance;
      return instance;
    }

    bool loadFromFile(const std::string& filepath);

    // bool saveToFile(const std::string& filepath);

    GraphicsSettings& getGraphicsSettings() { return graphics_settings_; }

  private:
    mutable std::shared_mutex settings_mutex_;

    GraphicsSettings graphics_settings_;

    bool graphicsFromJson(const json& j, GraphicsSettings& gs);

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    SettingsManager() = default;
  };
}
