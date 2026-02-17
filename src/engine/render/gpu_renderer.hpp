#ifndef GPU_RENDERER_HPP_
#define GPU_RENDERER_HPP_

#include <string>
#include <optional>

struct SDL_GPUDevice;
struct SDL_GPUShader;
struct SDL_GPUBuffer;
struct SDL_GPUTransferBuffer;
struct SDL_GPUGraphicsPipeline;
struct SDL_GPUCommandBuffer;
struct SDL_Window;

namespace engine::core {
    class Context;
}

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {

    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    class GPURenderer final {
    public:
        GPURenderer(SDL_GPUDevice* device, SDL_Window* window);

        GPURenderer(const GPURenderer&) = delete;
        GPURenderer& operator=(const GPURenderer&) = delete;
        GPURenderer(GPURenderer&&) = delete;
        GPURenderer& operator=(GPURenderer&&) = delete;

        SDL_GPUDevice* getGPUDevice() const { return device_; }

        void init();
        void render();

    private:
        SDL_GPUDevice* device_ = nullptr;
        SDL_Window* window_ = nullptr;
        SDL_GPUBuffer* vertex_buffer_ = nullptr;
        SDL_GPUBuffer* fragment_buffer_ = nullptr;
        SDL_GPUTransferBuffer* transfer_buffer_ = nullptr;
        SDL_GPUGraphicsPipeline* graphics_pipeline_ = nullptr;

        SDL_GPUShader* initVertexShader();
        SDL_GPUShader* initFragmentShader();

    };

} // namespace engine::render

#endif // GPU_RENDERER_HPP_
