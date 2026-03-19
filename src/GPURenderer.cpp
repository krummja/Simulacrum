#include "GPURenderer.hpp"
#include "ResourcePath.hpp"
#include "GPUShaderManager.hpp"
#include <cstring>
#include <format>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace Simulacrum
{
  bool GPURenderer::init()
  {
    if (is_initialized_)
    {
      spdlog::warn("GPURenderer already initialized");
      return true;
    }

    auto& gpu_device = GPUDevice::Instance();

    device_ = gpu_device.get();
    window_ = gpu_device.getWindow();

    int w = 0;
    int h = 0;

    SDL_GetWindowSize(window_, &w, &h);
    viewport_width_ = static_cast<Uint32>(w);
    viewport_height_ = static_cast<Uint32>(h);

    if (!GPUShaderManager::Instance().init(device_))
    {
      spdlog::error("GPURenderer: failed to initialize shader manager.");
      return false;
    }

    // Create samplers

    // Create scene texture

    // Load shaders

    // Create pipelines

    // Initialize vertex pools

    // Initialize sprite batches

    is_initialized_ = true;
    return true;
  }

  void GPURenderer::shutdown()
  {

  }

  SDL_GPURenderPass* GPURenderer::beginScenePass()
  {
    return nullptr;
  }

  SDL_GPURenderPass* GPURenderer::beginSwapchainPass()
  {
    return nullptr;
  }

  void GPURenderer::beginFrame()
  {

  }

  void GPURenderer::endFrame()
  {

  }
}
