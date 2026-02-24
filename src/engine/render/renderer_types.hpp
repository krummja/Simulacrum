#include "SDL3/SDL.h"
#include "SDL3/SDL_gpu.h"

namespace engine::render {

    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    struct UniformBuffer {
        float time;
    };

    struct MeshGPU {
        SDL_GPUBuffer* vertex_buffer = nullptr;
        Uint32 vertex_offset_bytes = 0;

        SDL_GPUBuffer* fragment_buffer = nullptr;
        Uint32 index_offset_bytes = 0;
        Uint32 index_count = 0;
        bool indexed = false;

        Uint32 vertex_count = 0;
    };

} // namespace engine::render
