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
        static GPURenderer& Instance();

        /// @brief Initialize the renderer.
        /// Must be called after GPUDevice::init().
        /// @return true on success
        bool init();

        /// @brief Shutdown and release all resources.
        void shutdown();

        /// @brief Begin a new frame.
        /// - Acquires command buffer
        /// - Begins copy pass for uploads
        void beginFrame();

        /// @brief End copy pass and begin scene render pass.
        /// @return Active render pass for scene rendering
        SDL_GPURenderPass* beginScenePass();

        /// @brief End scene pass and begin swapchain pass.
        /// @return Active render pass for UI/final compositing
        SDL_GPURenderPass* beginSwapchainPass();

        /// @brief End current frame.
        /// - Ends active render frame
        /// - Submits command buffer
        void endFrame();

        SDL_GPUGraphicsPipeline* getSpriteOpaquePipeline() const;
        SDL_GPUGraphicsPipeline* getSpriteAlphaPipeline() const;
        SDL_GPUGraphicsPipeline* getParticlePipeline() const;
        SDL_GPUGraphicsPipeline* getPrimitivePipeline() const;
        SDL_GPUGraphicsPipeline* getCompositePipeline() const;

        SDL_GPUGraphicsPipeline* getUISpritePipeline() const;
        SDL_GPUGraphicsPipeline* getUIPrimitivePipeline() const;

        SDL_GPUSampler* getNearestSampler() const { return nearest_sampler_.get(); }
        SDL_GPUSampler* getLinearSampler() const { return linear_sampler_.get(); }

        GPUTexture* getSceneTexture() const { return scene_texture_.get(); }

        GPUVertexPool& getSpriteVertexPool() { return sprite_vertex_pool_; }
        GPUVertexPool& getEntityVertexPool() { return entity_vertex_pool_; }
        GPUVertexPool& getParticleVertexPool() { return particle_vertex_pool_; }
        GPUVertexPool& getPrimitiveVertexPool() { return primitive_vertex_pool_; }
        GPUVertexPool& getUIVertexPool() { return ui_vertex_pool_; }

        SpriteBatch& getSpriteBatch() { return sprite_batch_; }
        SpriteBatch& getEntityBatch() { return entity_batch_; }

        SDL_GPUCommandBuffer* getCommandBuffer() const { return command_buffer_; }

        SDL_GPUCopyPass* getCopyPass() const { return copy_pass_; }

        uint32_t getViewportWidth() const { return viewport_width_; }
        uint32_t getViewportHeight() const { return viewport_height_; }

        /// @brief Update viewport dimensions (e.g. on window resize).
        void updateViewport(uint32_t width, uint32_t height);

        /// @brief Push view-projection matrix uniform.
        /// @param pass Active render pass
        /// @param view_projection 4x4 matrix data
        void pushViewProjection(SDL_GPURenderPass* pass, const float* view_projection);

        /// @brief Push composite uniforms.
        void pushCompositeUniforms(
            SDL_GPURenderPass* pass,
            float subpixelX,
            float subpixelY,
            float zoom
        );

        /// @brief Set composite parameters for the current frame.
        /// Call this during recordGPUVertices to configure zoom/scrolling.
        /// @param zoom Zoom level (1.0 = no zoom)
        /// @param subpixelX Subpixel X offset for smooth scrolling
        /// @param subpixelY Subpixel Y offset for smooth scrolling
        void setCompositeParams(float zoom, float subpixelX = 0.0f, float subpixelY = 0.0f);

        /// @brief Render the scene texture to the swapchain with compositing.
        /// Uses composite params set via setCompositeParams().
        /// @param pass Active swapchain render pass
        void renderComposite(SDL_GPURenderPass* pass);

        /// @brief Create an orthographic projection matrix for 2D rendering
        /// @param left Left coordinate
        /// @param right Right coordinate
        /// @param bottom Bottom coordinate
        /// @param top Top coordinate
        /// @param out Output matrix (16 floats)
        static void createOrthoMatrix(
            float left,
            float right,
            float bottom,
            float top,
            float* out
        );

    private:
        GPURenderer() = default;
        ~GPURenderer() = default;

        // Non-copyable
        GPURenderer(const GPURenderer&) = delete;
        GPURenderer& operator=(const GPURenderer&) = delete;

        bool loadShaders();
        bool createPipelines();
        bool createSceneTexture();
        void cleanupPartialInit();

        // Device reference
        SDL_GPUDevice* device_{nullptr};
        SDL_Window* window_{nullptr};

        // Frame state
		SDL_GPUCommandBuffer* command_buffer_{nullptr};
        SDL_GPUCopyPass* copy_pass_{nullptr};
        SDL_GPURenderPass* render_pass_{nullptr};

        // Swapchain state (acquired in beginFrame for authoritative dimensions)
        SDL_GPUTexture* swapchain_texture_{nullptr};
        uint32_t swapchain_width_{0};
        uint32_t swapchain_height{0};

        // Intermediate scene texture
        std::unique_ptr<GPUTexture> scene_texture_;

        // Samplers
        GPUSampler nearest_sampler_;
        GPUSampler linear_sampler_;

        // Pipelines (scene rendering - to scene texture)
        GPUPipeline sprite_opaque_pipeline_;
        GPUPipeline sprite_alpha_pipeline_;
        GPUPipeline particle_pipeline_;
        GPUPipeline primitive_pipeline_;
        GPUPipeline composite_pipeline_;

        // Pipelines (UI rendering - to swapchain)
        GPUPipeline ui_sprite_pipeline_;
        GPUPipeline ui_primitive_pipeline_;

        // Vertex pools
        GPUVertexPool sprite_vertex_pool_;
        GPUVertexPool entity_vertex_pool_;
        GPUVertexPool particle_vertex_pool_;
        GPUVertexPool primitive_vertex_pool_;
        GPUVertexPool ui_vertex_pool_;

        // Sprite batches
        SpriteBatch sprite_batch_;
        SpriteBatch entity_batch_;

        // Viewport (initialized from window size in init())
        uint32_t viewport_width_{0};
        uint32_t viewport_height_{0};

        // Composite params (set per-frame by game state)
        float composite_zoom_{1.0f};
        float composite_subpixel_x_{0.0f};
        float composite_subpixel_y_{0.0f};

        // Debug tracking for scene texture dimension changes (avoids static variables)
        uint32_t last_logged_scene_w_{0};
        uint32_t last_logged_scene_h_{0};

        bool initialized_{false};
    };

} // namespace engine::render
