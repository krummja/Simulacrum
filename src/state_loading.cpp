#include "state_loading.hpp"
#include "simulacrum_engine.hpp"
#include "thread_system.hpp"
#include "state_manager.hpp"
#include "gpu_renderer.hpp"
#include "gpu_types.hpp"

#include <format>

namespace Simulacrum {

    void LoadingState::configure(const std::string& target_state_name) {
        target_state_name_ = target_state_name;
    }

    bool LoadingState::enter() {
        spdlog::debug("Entering Loading State");
        return true;
    }

    void LoadingState::update([[maybe_unused]] float delta_time) {}

    void LoadingState::render(SDL_Renderer* renderer, float /* interpolation_alpha */) {}

    void LoadingState::handleInput() {}

    void LoadingState::exit() {}

    std::string LoadingState::getName() const { return "Loading State"; }

    void LoadingState::recordGPUVertices(GPURenderer& gpu_renderer, float interpolation_alpha) {
        (void)interpolation_alpha;

        auto& vertex_pool = gpu_renderer.getPrimitiveVertexPool();

        // The mapped pointer on the vertex pool is created in GPUVertexPool::beginFrame()
        // During GPURenderer::beginScenePass(), the vertex pool's upload method is called
        // using the scene copy pass. This uploads the contents of the mapped pointer to
        // the GPU.
        auto* write_ptr = static_cast<ColorVertex*>(vertex_pool.getMappedPtr());

        if (!write_ptr) {
            return;
        }

        ColorVertex* v = write_ptr;
        // Vertex 0: top-left
        v[0] = {0.0f, 0.0f, 255, 255, 255, 255};
        // Vertex 1: top-right
        v[1] = {100.0f, 0.0f, 255, 255, 255, 255};
        // Vertex 2: bottom-right
        v[2] = {100.0f, 100.0f, 255, 255, 255, 255};
        // Vertex 3: bottom-left
        v[3] = {0.0f, 100.0f, 255, 255, 255, 255};
    }

    void LoadingState::renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass) {}

} // namespace Simulacrum
