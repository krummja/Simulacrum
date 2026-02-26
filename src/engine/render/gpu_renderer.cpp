#include "gpu_renderer.hpp"
#include "gpu_shader_manager.hpp"
#include <cstring>
#include <format>
#include <spdlog/spdlog.h>

namespace engine::render {

    GPURenderer& GPURenderer::Instance() {
        static GPURenderer instance;
        return instance;
    }

    bool GPURenderer::init() {
        if (initialized_) {
            spdlog::warn("GPURenderer already initialized");
            return true;
        }

        auto& gpu_device = GPUDevice::Instance();
        if (!gpu_device.isInitialized()) {
            spdlog::error("GPURenderer::init: GPUDevice not initialized");
            return false;
        }

        device_ = gpu_device.get();
        window_ = gpu_device.getWindow();

        // Get window size for viewport (logical size - matches swapchain)
        int w = 0;
        int h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        viewport_width_ = static_cast<uint32_t>(w);
        viewport_height_ = static_cast<uint32_t>(h);

        // Initialize shader manager
        if (!GPUShaderManager::Instance().init(device_)) {
            spdlog::error("GPURenderer: failed to init shader manager");
            return false;
        }

        // Create samplers
        nearest_sampler_ = GPUSampler::createNearest(device_);
        linear_sampler_ = GPUSampler::createLinear(device_);

        if (!nearest_sampler_.isValid() || !linear_sampler_.isValid()) {
            spdlog::error("GPURenderer: failed to create samplers");
            cleanupPartialInit();
            return false;
        }

        // Create scene texture
        if (!createSceneTexture()) {
            spdlog::error("GPURenderer: failed to create scene texture");
            cleanupPartialInit();
            return false;
        }

        // Load shader and create pipelines
        if (!loadShaders()) {
            spdlog::error("GPURenderer: failed to create shaders");
            cleanupPartialInit();
            return false;
        }

        if (!createPipelines()) {
            spdlog::error("GPURenderer: failed to create pipelines");
            cleanupPartialInit();
            return false;
        }

        // Initialize vertex pools
        if (!primitive_vertex_pool_.init(device_, sizeof(ColorVertex), 300000)) {
            spdlog::error("GPURenderer: failed to init primitive vertex pool");
            cleanupPartialInit();
            return false;
        }

        if (!ui_vertex_pool_.init(device_, sizeof(SpriteVertex), 4000)) {
            spdlog::error("GPURenderer: failed to init UI vertex pool");
            cleanupPartialInit();
            return false;
        }

        initialized_ = true;
        spdlog::info("GPURenderer initialized: {}x{}", viewport_width_, viewport_height_);
        return true;
    }

    void GPURenderer::shutdown() {
        if (!initialized_) {
            return;
        }

        // Release vertex pools
        primitive_vertex_pool_.shutdown();
        ui_vertex_pool_.shutdown();

        // Release pipelines
        primitive_pipeline_.release();
        ui_sprite_pipeline_.release();
        ui_primitive_pipeline_.release();

        // Release textures and samplers
        scene_texture_.reset();
        nearest_sampler_ = GPUSampler();
        linear_sampler_ = GPUSampler();

        // Shutdown shader manager
        GPUShaderManager::Instance().shutdown();

        device_ = nullptr;
        window_ = nullptr;
        initialized_ = false;

        spdlog::info("GPURenderer shutdown complete");
    }

    void GPURenderer::cleanupPartialInit() {
        // Release vertex pools
        primitive_vertex_pool_.shutdown();
        ui_vertex_pool_.shutdown();

        // Release pipelines
        primitive_pipeline_.release();
        ui_sprite_pipeline_.release();
        ui_primitive_pipeline_.release();

        // Release textures and samplers
        scene_texture_.reset();
        nearest_sampler_ = GPUSampler();
        linear_sampler_ = GPUSampler();

        // Shutdown shader manager
        GPUShaderManager::Instance().shutdown();

        device_ = nullptr;
        window_ = nullptr;
    }

    void GPURenderer::beginFrame() {

    }

    SDL_GPURenderPass* GPURenderer::beginScenePass() {
        if (!command_buffer_) {
            return nullptr;
        }
    }

    SDL_GPURenderPass* GPURenderer::beginSwapchainPass() {
        if (!command_buffer_) {
            return nullptr;
        }
    }

    void GPURenderer::endFrame() {

    }

    SDL_GPUGraphicsPipeline* GPURenderer::getPrimitivePipeline() const {
        return primitive_pipeline_.get();
    }

    SDL_GPUGraphicsPipeline* GPURenderer::getUISpritePipeline() const {
        return ui_sprite_pipeline_.get();
    }

    SDL_GPUGraphicsPipeline* GPURenderer::getUIPrimitivePipeline() const {
        return ui_primitive_pipeline_.get();
    }

    void GPURenderer::updateViewport(uint32_t width, uint32_t height) {

    }

    void GPURenderer::pushViewProjection(SDL_GPURenderPass* pass, const float* view_projection) {

    }

    void GPURenderer::pushCompositeUniforms(SDL_GPURenderPass* pass, float subpixelX, float subpixelY, float zoom) {

    }

    bool GPURenderer::loadShaders() {
        return true;
    }

} // namespace engine::render
