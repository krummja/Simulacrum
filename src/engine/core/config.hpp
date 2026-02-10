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

private:

};

} // namespace engine::core
