#include "state_manager.hpp"

namespace Simulacrum {

    StateManager::StateManager() {
        registered_states_.reserve(8);
        active_states_.reserve(3);
    }

} // namespace Simulacrum
