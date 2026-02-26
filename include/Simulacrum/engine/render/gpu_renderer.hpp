#pragma once

#include "gpu_types.hpp"
#include "gpu_device.hpp"
#include "gpu_texture.hpp"
#include "gpu_sampler.hpp"
#include "gpu_buffer.hpp"
#include "gpu_transfer_buffer.hpp"
#include "gpu_pipeline.hpp"
#include "gpu_vertex_pool.hpp"
#include "sprite_batch.hpp"
#include <SDL3/SDL_gpu.h>
#include <memory>
#include <vector>

namespace engine::render {

    /// @brief Main GPU renderer singleton.
    ///
    /// Orchestrates the rendering pipeline:
    /// - Command buffer management
    /// - Copy pass for uploads
    /// - Render passes for scene and swapchain
    /// - Pipeline state management
    class GPURenderer {
    public:

    private:
        GPURenderer() = default;
        ~GPURenderer() = default;

        // Non-copyable
        GPURenderer(const GPURenderer&) = delete;
        GPURenderer& operator=(const GPURenderer&) = delete;

        bool loadShaders();
        bool createPipelines();
        bool createSceneTexture();
        bool cleanupPartialInit();
    };

} // namespace engine::render
