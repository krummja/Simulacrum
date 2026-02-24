#pragma once

#include "draw_queue.hpp"
#include <string>
#include <optional>
#include <SDL3/SDL_gpu.h>

namespace engine::render {

    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    struct UniformBuffer {
        float time;
    };

    class GPURenderer final {
    public:
        GPURenderer(SDL_GPUDevice* device, SDL_Window* window);

        // Copy operations
        GPURenderer(const GPURenderer&) = delete;
        GPURenderer& operator=(const GPURenderer&) = delete;

        // Move operations
        GPURenderer(GPURenderer&&) = delete;
        GPURenderer& operator=(GPURenderer&&) = delete;

        SDL_GPUDevice* getGPUDevice() const { return device_; }

        void init();
        void render();
        void close();
        void submit(MeshGPU* mesh);

    private:
        std::unique_ptr<DrawQueue> draw_queue_ = nullptr;

        SDL_GPUDevice* device_ = nullptr;
        SDL_Window* window_ = nullptr;
        SDL_GPUBuffer* vertex_buffer_ = nullptr;
        SDL_GPUBuffer* fragment_buffer_ = nullptr;
        SDL_GPUTransferBuffer* transfer_buffer_ = nullptr;
        SDL_GPUGraphicsPipeline* graphics_pipeline_ = nullptr;

        SDL_GPUShader* initVertexShader();
        SDL_GPUShader* initFragmentShader();
        SDL_GPUColorTargetInfo createColorTarget(SDL_GPUTexture* swapchain_texture);

        void initGraphicsPipeline();
        void initVertexBuffer();
        void initTransferBuffer();
        void fillTransferBuffer();
        void executeCopyPass();
    };
} // namespace engine::render
