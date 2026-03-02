#pragma once

#include "macros.hpp"
#include "state.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct SDL_GPURenderPass;
class GPURenderer;

namespace Simulacrum {

    class Simulacrum_API StateManager {
    public:
        StateManager();

        // void addState(std::unique_ptr<Simulacrum::State> state);
        // void pushState(const std::string& state_name);
        // void popState();
        // void changeState(const std::string& state_name);

        // void update(float delta_time);
        // void render(SDL_Renderer* renderer, float interpolationAlpha = 1.0f);
        // void handleInput();

        // void recordGPUVertices(GPURenderer& gpu_renderer, float interpolation_alpha);
        // void renderGPUScene(GPURenderer& renderer, SDL_GPURenderPass* scene_pass, float interpolation_alpha);
        // void renderGPUUIScene(GPURenderer& renderer, SDL_GPURenderPass* swapchain_pass);

        // bool hasState(const std::string& state_name) const;
        // std::shared_ptr<Simulacrum::State> getState(const std::string& state_name) const;
        // void removeState(const std::string& state_name);
        // void clearAllStates();

        // void setCurrentFPS(float fps) { current_fps_ = fps; }
        // float getCurrentFPS() const { return current_fps_; }

    private:
        std::unordered_map<std::string, std::shared_ptr<Simulacrum::State>> registered_states_;
        std::vector<std::shared_ptr<Simulacrum::State>> active_states_;

        float last_delta_time_{0.0f};
        float current_fps_{0.0f};

    };

} // namespace Simulacrum
