#include "StateLoading.hpp"
#include "SimulacrumEngine.hpp"
#include "ThreadSystem.hpp"
#include "StateManager.hpp"
#include "GpuRenderer.hpp"
#include "GpuTypes.hpp"
#include "UIManager.hpp"

#include <format>

namespace Simulacrum
{

  void LoadingState::configure(const std::string& target_state_name)
  {
    target_state_name_ = target_state_name;
  }

  bool LoadingState::enter()
  {
    spdlog::debug("Entering Loading State");

    auto& ui = UIManager::Instance();
    ui.createPanel("panel-1", {-1, 0, 300, 300});

    return true;
  }

  void LoadingState::update([[maybe_unused]] float delta_time) {}

  void LoadingState::render(SDL_Renderer* renderer, float /* interpolation_alpha */) {}

  void LoadingState::handleInput() {}

  void LoadingState::exit() {}

  std::string LoadingState::getName() const { return "Loading State"; }

  void LoadingState::recordGPUVertices(GPURenderer& gpu_renderer, float interpolation_alpha)
  {
    (void)interpolation_alpha;

    auto& ui = UIManager::Instance();
    ui.recordGPUVertices(gpu_renderer);
  }

  void LoadingState::renderGPUScene(
    GPURenderer& gpu_renderer,
    SDL_GPURenderPass* scene_pass,
    float interpolation_alpha
  )
  {}

  void LoadingState::renderGPUUI(
    GPURenderer& gpu_renderer,
    SDL_GPURenderPass* swapchain_pass
  )
  {
    auto& ui = UIManager::Instance();
    ui.renderGPU(gpu_renderer, swapchain_pass);
  }

} // namespace Simulacrum
