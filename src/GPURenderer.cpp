#include "GPURenderer.hpp"

namespace Simulacrum
{
  bool GPURenderer::init()
  {
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
