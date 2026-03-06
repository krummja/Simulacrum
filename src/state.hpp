#pragma once

#include "macros.hpp"
#include <string>

struct SDL_GPURenderPass;
struct SDL_Renderer;

namespace Simulacrum
{
  class StateManager;
  class GPURenderer;

  class State
  {
  public:
    virtual bool enter() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(SDL_Renderer* renderer, float interpolation_alpha = 1.0f) = 0;
    virtual void handleInput() = 0;
    virtual void exit() = 0;
    virtual void pause() {}
    virtual void resume() {}
    virtual std::string getName() const = 0;

    virtual ~State() = default;

    /// @brief Record vertices for GPU rendering (called before scene pass).
    /// Override in states that support GPU rendering.
    /// @param gpu_renderer Reference to GPURenderer for accessing vertex pools/batches
    /// @param interpolation_alpha Interpolation factor for smooth rendering
    virtual void recordGPUVertices(
      [[maybe_unused]] GPURenderer& gpu_renderer,
      [[maybe_unused]] float interpolation_alpha
    )
    {}

    /// @brief Issue GPU draw calls during scene pass.
    /// Override in states that support GPU rendering.
    /// @param gpu_renderer Reference to GPURenderer
    /// @param scene_pass Active scene render pass
    /// @param interpolation_alpha Interpolation factor
    virtual void renderGPUScene(
      [[maybe_unused]] GPURenderer& gpu_renderer,
      [[maybe_unused]] SDL_GPURenderPass* scene_pass,
      [[maybe_unused]] float interpolation_alpha
    )
    {}

    /// @brief Render UI/overlays during swapchain pass.
    /// Overrides in states that need to render UI with GPU.
    /// UI renders at exact screen positions; no interpolation needed.
    /// @param gpu_renderer Reference to GPURenderer
    /// @param swapchain_pass Active swapchain render pass
    virtual void renderGPUUI(
      [[maybe_unused]] GPURenderer& gpu_renderer,
      [[maybe_unused]] SDL_GPURenderPass* swapchain_pass
    )
    {}

    void setStateManager(StateManager* manager) { state_manager_ = manager; }

  protected:
    StateManager* state_manager_{ nullptr };
  };

} // namespace Simulacrum
