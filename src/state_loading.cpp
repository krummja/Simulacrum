#include "state_loading.hpp"
#include "simulacrum_engine.hpp"
#include "thread_system.hpp"
#include "state_manager.hpp"
#include "gpu_renderer.hpp"

#include <format>

namespace Simulacrum {

    void LoadingState::configure(const std::string& target_state_name) {
        target_state_name_ = target_state_name;
    }

    bool LoadingState::enter() {
        return true;
    }

    void LoadingState::update([[maybe_unused]] float delta_time) {

    }

    void LoadingState::render(SDL_Renderer* renderer, float /* interpolation_alpha */) {

    }

    void LoadingState::handleInput() {}

    void LoadingState::exit() {}

    std::string LoadingState::getName() const { return "Loading State"; }

    void LoadingState::recordGPUVertices(Simulacrum::GPURenderer& gpu_renderer, float interpolation_alpha) {

    }

    void LoadingState::renderGPUUI(Simulacrum::GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass) {

    }

} // namespace Simulacrum
