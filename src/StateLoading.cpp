#include "StateLoading.hpp"
#include "SimulacrumEngine.hpp"
#include "TextureManager.hpp"
#include "ThreadSystem.hpp"
#include "StateManager.hpp"
#include "GpuRenderer.hpp"
#include "GpuTypes.hpp"
#include "GpuTexture.hpp"
#include "UIManager.hpp"

#include <format>
#include <cmath>
#include <string>
#include <cstring>

namespace Simulacrum
{
  void LoadingState::configure(const std::string& target_state_name)
  {
    target_state_name_ = target_state_name;
  }

  bool LoadingState::enter()
  {
    spdlog::debug("Entering Loading State");
    spdlog::debug("{}", SimulacrumEngine::Instance().getWindowWidth());

    auto& ui = UIManager::Instance();

    int left_column_x = 50;
    int left_column_w = 220;
    int right_column_x = ui.getLogicalWidth() / 2 + 50;
    int right_column_w = ui.getLogicalWidth() - right_column_x - 50;

    ui.createLabel(
      "example_label",
      {
        150,
        ui.getLogicalHeight() - 75,
        200,
        30
      },
      "Hello world!"
    );

    return true;
  }

  void LoadingState::update([[maybe_unused]] float delta_time) {}

  void LoadingState::render(SDL_Renderer* renderer, float /* interpolation_alpha */) {}

  void LoadingState::handleInput() {}

  void LoadingState::exit() {}

  std::string LoadingState::getName() const { return "Loading State"; }

  void LoadingState::recordGPUVertices(GPURenderer& gpu_renderer, [[maybe_unused]] float interpolation_alpha)
  {
    auto& ui = UIManager::Instance();
    if (!ui.isShutdown())
    {
      ui.recordGPUVertices(gpu_renderer);
    }
  }

  void LoadingState::renderGPUScene(
    GPURenderer& gpu_renderer,
    SDL_GPURenderPass* scene_pass,
    float interpolation_alpha
  )
  {
  }

  void LoadingState::renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass)
  {
    auto& ui = UIManager::Instance();

    if (!ui.isShutdown())
    {
      ui.renderGPU(gpu_renderer, swapchain_pass);
    }
  }

} // namespace Simulacrum
