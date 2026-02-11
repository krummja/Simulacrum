#include "context.hpp"
#include "../input/input_manager.hpp"
#include <spdlog/spdlog.h>

namespace engine::core {

    Context::Context(engine::input::InputManager& input_manager)
    : input_manager_(input_manager) {}

} // namespace engine::core
