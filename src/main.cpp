#include <spdlog/spdlog.h>

#include "Simulacrum/engine/render/gpu_renderer.hpp"


int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::trace);
    engine::render::GPURenderer& renderer = engine::render::GPURenderer::Instance();
    renderer.init();
    return 0;
}
