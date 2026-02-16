#include "gpu_renderer.hpp"
#include "../resource/resource_manager.hpp"
#include "../core/context.hpp"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace engine::render {

    GPURenderer::GPURenderer(SDL_GPUDevice* device): device_(device) {}

} // namespace engine::render
