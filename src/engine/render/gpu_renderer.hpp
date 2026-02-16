#ifndef GPU_RENDERER_HPP_
#define GPU_RENDERER_HPP_

#include <string>
#include <optional>

struct SDL_GPUDevice;
struct SDL_Window;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {

    class GPURenderer final {
    public:
        GPURenderer(SDL_GPUDevice* device);

        GPURenderer(const GPURenderer&) = delete;
        GPURenderer& operator=(const GPURenderer&) = delete;
        GPURenderer(GPURenderer&&) = delete;
        GPURenderer& operator=(GPURenderer&&) = delete;

        SDL_GPUDevice* getGPUDevice() const { return device_; }

    private:
        SDL_GPUDevice* device_ = nullptr;

    };

} // namespace engine::render

#endif // GPU_RENDERER_HPP_
