#ifndef SIMULACRUM_GPU_RENDERER_HPP_
#define SIMULACRUM_GPU_RENDERER_HPP_

#include "GPUDevice.hpp"

#include <SDL3/SDL_gpu.h>
#include <memory>

namespace Simulacrum
{
  typedef struct PositionVertex
  {
    float x, y, z;
  } PositionVertex;

  typedef struct PositionColorVertex
  {
    float x, y, z;
    Uint8 r, g, b, a;
  } PositionColorVertex;

  typedef struct PositionTextureVertex
  {
    float x, y, z;
    float u, v;
  } PositionTextureVertex;

  class GPURenderer
  {
  public:
    static GPURenderer& Instance()
    {
      static GPURenderer instance;
      return instance;
    }

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

  private:
    GPURenderer() = default;
    ~GPURenderer() = default;

    // No-copy
    GPURenderer(const GPURenderer&) = delete;
    GPURenderer& operator=(const GPURenderer&) = delete;

    SDL_GPUDevice* device_{ nullptr };
    SDL_Window* window_{ nullptr };

    SDL_GPUGraphicsPipeline* pipeline_{ nullptr };
    SDL_GPUTransferBuffer* transfer_buffer_{ nullptr };
    SDL_GPUSampler* sampler_{ nullptr };
    SDL_GPUBuffer* vertex_buffer_{ nullptr };
    SDL_GPUBuffer* index_buffer_{ nullptr };
    SDL_GPUCommandBuffer* command_buffer_{ nullptr };
    SDL_GPUCopyPass* copy_pass_{ nullptr };
    SDL_GPURenderPass* render_pass_{ nullptr };

    SDL_GPUTexture* scene_texture_{ nullptr };
    SDL_GPUTexture* swapchain_texture_{ nullptr };
    SDL_GPUTexture* test_texture_{ nullptr };
    Uint32 swapchain_width_{ 0 };
    Uint32 swapchain_height_{ 0 };

    Uint32 viewport_width_{ 0 };
    Uint32 viewport_height_{ 0 };
    bool is_initialized_{false};
  };
}

#endif