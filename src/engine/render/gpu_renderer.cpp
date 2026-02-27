#include "Simulacrum/engine/render/gpu_renderer.hpp"
#include <spdlog/spdlog.h>

namespace engine::render {

    GPURenderer& GPURenderer::Instance() {
        static GPURenderer instance;
        return instance;
    }

    void GPURenderer::init() {
        spdlog::debug("Hello from GPURenderer");
    }

    bool GPURenderer::loadShaders() {
        return true;
    }

    bool GPURenderer::createPipelines() {
        return true;
    }

    bool GPURenderer::createSceneTexture() {
        return true;
    }

    bool GPURenderer::cleanupPartialInit() {
        return true;
    }

} // namespace engine::render
