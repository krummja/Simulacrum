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
            throw std::runtime_error("State name already exists");
        }

        state->setStateManager(this);
        registered_states_[name] = std::shared_ptr<State>(state.release());
    }

    void StateManager::pushState(const std::string& state_name) {
        auto it = registered_states_.find(state_name);
        if (it != registered_states_.end()) {

            // Pause the current stop state if it exists
            if (!active_states_.empty()) {
                active_states_.back()->pause();
            }

            auto new_state = it->second;
            if (!new_state->enter()) {
                spdlog::error("Failed to enter state: {}", state_name);
                return;
            }

            active_states_.push_back(new_state);
        } else {
            spdlog::error("State not found: {}", state_name);
        }
    }

    void StateManager::popState() {
        if (!active_states_.empty()) {
            auto current_state = active_states_.back();
            current_state->exit();

            active_states_.pop_back();

            if (!active_states_.empty()) {
                active_states_.back()->resume();
            }
        }
    }

    void StateManager::changeState(const std::string& state_name) {
        if (!active_states_.empty()) {
            popState();
        }
        pushState(state_name);
    }

    void StateManager::update(float delta_time) {
        last_delta_time_ = delta_time;

        if (!active_states_.empty()) {
            active_states_.back()->update(delta_time);
        }
    }

    void StateManager::render(SDL_Renderer* renderer, float interpolation_alpha) {
        if (!active_states_.empty()) {
            active_states_.back()->render(renderer, interpolation_alpha);
        }
    }

    void StateManager::recordGPUVertices(GPURenderer& gpu_renderer, float interpolation_alpha) {
        if (!active_states_.empty()) {
            active_states_.back()->recordGPUVertices(gpu_renderer, interpolation_alpha);
        }
    }

    void StateManager::renderGPUScene(GPURenderer& gpu_renderer, SDL_GPURenderPass* scene_pass, float interpolation_alpha) {
        if (!active_states_.empty()) {
            active_states_.back()->renderGPUScene(gpu_renderer, scene_pass, interpolation_alpha);
        }
    }

    void StateManager::renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass) {
        if (!active_states_.empty()) {
            active_states_.back()->renderGPUUI(gpu_renderer, swapchain_pass);
        }
    }

    void StateManager::handleInput() {
        if (!active_states_.empty()) {
            active_states_.back()->handleInput();
        }
    }

    bool StateManager::hasState(const std::string& state_name) const {
        return registered_states_.find(state_name) != registered_states_.end();
    }

    std::shared_ptr<State> StateManager::getState(const std::string& state_name) const {
        auto it = registered_states_.find(state_name);
        return it != registered_states_.end() ? it->second : nullptr;
    }

    void StateManager::removeState(const std::string& state_name) {
        active_states_.erase(
            std::remove_if(
                active_states_.begin(),
                active_states_.end(),
                [&](const std::shared_ptr<State>& state) {
                    if (state->getName() == state_name) {
                        state->exit();
                        return true;
                    }
                    return false;
                }
            ),
            active_states_.end()
        );
    }

    void StateManager::clearAllStates() {
        for (const auto& state : active_states_) {
            state->exit();
        }
        active_states_.clear();
        registered_states_.clear();
    }

} // namespace Simulacrum
