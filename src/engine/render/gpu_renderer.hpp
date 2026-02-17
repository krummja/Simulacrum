#ifndef GPU_RENDERER_HPP_
#define GPU_RENDERER_HPP_

#include <string>
#include <optional>

struct SDL_GPUDevice;
struct SDL_GPUCommandBuffer;
struct SDL_Window;

namespace engine::core {
    class Context;
}

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {

    class GPURenderer final {
    public:
        GPURenderer(SDL_GPUDevice* device, SDL_Window* window);

        GPURenderer(const GPURenderer&) = delete;
        GPURenderer& operator=(const GPURenderer&) = delete;
        GPURenderer(GPURenderer&&) = delete;
        GPURenderer& operator=(GPURenderer&&) = delete;

        SDL_GPUDevice* getGPUDevice() const { return device_; }

        void render();

    private:
        SDL_GPUDevice* device_ = nullptr;
        SDL_Window* window_ = nullptr;

    };

} // namespace engine::render

#endif // GPU_RENDERER_HPP_
