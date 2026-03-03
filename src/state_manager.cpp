#include "macros.hpp"
#include "state_manager.hpp"
#include "state.hpp"
#include "gpu_renderer.hpp"
#include <spdlog/spdlog.h>
#include <format>
#include <algorithm>
#include <stdexcept>
#include <SDL3/SDL.h>

namespace Simulacrum {

    StateManager::StateManager() {
        registered_states_.reserve(8);
        active_states_.reserve(3);
    }

    void StateManager::addState(std::unique_ptr<State> state) {
        const std::string name = state->getName();
        if (hasState(name)) {
            spdlog::error("State with name {} already exists", name);
            // throw std::runtime_error(std::format("State with name {} already exists", name));
        }
    }

} // namespace Simulacrum
