#include "settings_manager.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Simulacrum {

    bool SettingsManager::loadFromFile(const std::string& filepath) {
        std::ifstream f(filepath);
        json data = json::parse(f);

        std::unique_lock<std::shared_mutex> lock(settings_mutex_);

        for (const auto& [category_name, category_value] : data.items()) {
            if (!category_value.is_object()) {
                spdlog::warn("Category '{}' is not an object, skipping", category_name);
            }

            spdlog::debug(category_name);
            spdlog::debug(category_value);
        }

        return true;
    }

} // namespace Simulacrum
