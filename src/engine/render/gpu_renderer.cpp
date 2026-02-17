#include "gpu_renderer.hpp"
#include "../resource/resource_manager.hpp"
#include "../core/context.hpp"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace engine::render {

    GPURenderer::GPURenderer(
        SDL_GPUDevice* device,
        SDL_Window* window
    )
        : device_(device)
        , window_(window)
    {}

    void GPURenderer::render() {
        SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(device_);

        SDL_GPUTexture* texture;
        Uint32 width, height;
        SDL_WaitAndAcquireGPUSwapchainTexture(buffer, window_, &texture, &width, &height);

        SDL_GPUColorTargetInfo targetInfo{};
        targetInfo.clear_color = {255/255.0f, 0/255.0f, 255/255.0f, 255/255.0f};
        targetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        targetInfo.store_op = SDL_GPU_STOREOP_STORE;
        targetInfo.texture = texture;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(buffer, &targetInfo, 1, NULL);

        SDL_EndGPURenderPass(renderPass);

        SDL_SubmitGPUCommandBuffer(buffer);
    }

} // namespace engine::render
