#include "settings_manager.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Simulacrum
{

  bool SettingsManager::loadFromFile(const std::string& filepath)
  {
    std::ifstream f(filepath);
    json data = json::parse(f);

    std::unique_lock<std::shared_mutex> lock(settings_mutex_);

    GraphicsSettings graphics_settings{};

    for (const auto& [category_name, category_value] : data.items())
    {
      if (category_name == "graphics")
      {
        if (!graphicsFromJson(category_value, graphics_settings))
        {
          spdlog::error("Failed to load graphics settings");
        }

        graphics_settings_ = graphics_settings;
      }
    }

    return true;
  }

  bool SettingsManager::graphicsFromJson(const json& j, GraphicsSettings& gs)
  {
    j.at("vsync").get_to(gs.vsync);
    j.at("resolution_width").get_to(gs.resolution_width);
    j.at("resolution_height").get_to(gs.resolution_height);
    j.at("fullscreen").get_to(gs.fullscreen);
    j.at("buffer_count").get_to(gs.buffer_count);
    j.at("show_fps").get_to(gs.show_fps);
    j.at("fps_limit").get_to(gs.fps_limit);
    return true;
  }

} // namespace Simulacrum
