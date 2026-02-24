#include <SDL3/SDL_gpu.h>

#include "gpu_renderer.hpp"

namespace engine::render {

    struct DrawCommand {
        SDL_GPUGraphicsPipeline* pipeline;
        SDL_GPUBuffer* vertex_buffer;
        uint32_t vertex_count;
        uint32_t instance_count;
        uint32_t first_vertex;
        uint32_t first_instance;

        UniformBuffer uniforms;
        uint64_t sort_key;
    } DrawCommand;

} // namespace engine::render
