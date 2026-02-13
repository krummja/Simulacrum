#include "context.hpp"
#include "../input/input_manager.hpp"
#include <spdlog/spdlog.h>

namespace engine::core {

    Context::Context(
        engine::input::InputManager& input_manager,
        engine::core::GameState& game_state
    )
        : input_manager_(input_manager)
        , game_state_(game_state)
    {}

} // namespace engine::core
