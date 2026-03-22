#include "Camera.hpp"
#include "Entity.hpp"
#include "SimulacrumEngine.hpp"
#include "GpuRenderer.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <random>
// #include <format>

namespace Simulacrum
{

    Camera::Camera()
    {
        if (config_.isValid() && !config_.zoom_levels.empty())
        {
            current_zoom_index_ = config_.default_zoom_level;
            zoom_ = config_.zoom_levels[current_zoom_index_];
        }
    }

    Camera::Camera(const Config& config) : config_(config)
    {
        if (!config_.isValid())
        {
            spdlog::warn("Invalid camera configuration provided, using defaults");
            config_ = Config{};
        }

        if (config_.isValid() && !config_.zoom_levels.empty())
        {
            current_zoom_index_ = config_.default_zoom_level;
            zoom_ = config_.zoom_levels[current_zoom_index_];
        }
    }

} // namespace Simulacrum
