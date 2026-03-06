#pragma once

#include "State.hpp"
#include <atomic>
#include <future>
#include <string>

namespace Simulacrum
{

  class LoadingState : public State
  {
  public:
    LoadingState() = default;

    void configure(const std::string& target_state_name);

    bool enter() override;

    void update(float delta_time) override;

    void render(SDL_Renderer* renderer, float interpolation_alpha = 1.0f) override;

    void handleInput() override;

    void exit() override;

    std::string getName() const override;

    void recordGPUVertices(GPURenderer& gpu_renderer, float interpolation_alpha) override;

    void renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass) override;

  private:
    std::string target_state_name_;

  };

} // namespace Simulacrum
