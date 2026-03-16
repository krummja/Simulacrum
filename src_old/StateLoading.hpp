#pragma once

#include "State.hpp"
#include <future>
#include <string>
#include <vector>

// Forward reference
struct SDL_GPUTexture;

namespace Simulacrum
{
  class SpriteBatch;

  class LoadingState : public State
  {
  public:
    LoadingState() = default;

    void configure(const std::string& target_state_name);

    bool enter() override;

    void update(float delta_time) override;

    void render(SDL_Renderer* renderer, float interpolation_alpha) override;

    void handleInput() override;

    void exit() override;

    [[nodiscard]] std::string getName() const override;

    void recordGPUVertices(GPURenderer& gpu_renderer, float interpolation_alpha) override;

    void renderGPUScene(GPURenderer& gpu_renderer, SDL_GPURenderPass* scene_pass, float interpolation_alpha) override;

    void renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass) override;

  private:
    std::string target_state_name_;
    SpriteBatch* sprite_batch_{nullptr};

    struct GPUDrawCommand
    {
      SDL_GPUTexture* texture{nullptr};
      uint32_t vertex_offset{0};
      uint32_t vertex_count{0};
    };

    std::vector<GPUDrawCommand> primitive_commands_;
    std::vector<GPUDrawCommand> image_commands_;
  };

} // namespace Simulacrum
