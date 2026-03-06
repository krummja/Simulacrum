#pragma once

#include "gpu_types.hpp"
#include "gpu_buffer.hpp"
#include <SDL3/SDL_gpu.h>
#include <cstdint>
#include <vector>

namespace Simulacrum
{

  /// @brief Batched sprite renderer for GPU rendering.
  ///
  /// Works with GPURenderer's vertex pool system:
  /// 1. During `beginFrame()`, vertex pool is mapped
  /// 2. SpriteBatch writes sprites to the mapped buffer
  /// 3. During `beginScenePass()`, vertices are uploaded via copy pass
  /// 4. During render pass, draw calls are issued
  ///
  /// Usage:
  /// - Before render pass (during vertex recording phase):
  ///
  /// ```cpp
  /// batch.begin(texture, sampler, texWidth, textHeight);
  /// batch.draw(srcRect, dstRect);
  /// size_t vertex_count = batch.end();          // Returns vertex count for pool
  /// ...
  /// batch.render(pass, pipeline, vertex_pool);  // During render pass
  /// ```
  class SpriteBatch
  {
  public:
    static constexpr size_t MAX_SPRITES = 50000;
    static constexpr size_t VERTICES_PER_SPRITE = 4;
    static constexpr size_t INDICES_PER_SPRITE = 6;
    static constexpr size_t MAX_VERTICES = MAX_SPRITES * VERTICES_PER_SPRITE;
    static constexpr size_t MAX_INDICES = MAX_SPRITES * INDICES_PER_SPRITE;

    SpriteBatch() = default;
    ~SpriteBatch() = default;

    // No-copy
    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    // Movable (explicit to properly clear resource state)
    SpriteBatch(SpriteBatch&&) noexcept;
    SpriteBatch& operator=(SpriteBatch&&) noexcept;

    /// @brief Initialize the sprite batch.
    /// @param device GPU device
    /// @return true on success
    bool init(SDL_GPUDevice* device);

    /// @brief Shutdown and release resources.
    void shutdown();

    /// @brief Begin recording sprites (call before render pass).
    /// @param write_ptr Pointer to mapped vertex buffer (from vertex pool)
    /// @param max_vertices Maximum vertices that can be written
    /// @param texture Texture for the batch
    /// @param sampler Sampler for the batch
    /// @param texture_width Width of texture in pixels
    /// @param texture_height Height of texture in pixels
    void begin(
      SpriteVertex* write_ptr,
      size_t max_vertices,
      SDL_GPUTexture* texture,
      SDL_GPUSampler* sampler,
      float texture_width,
      float texture_height
    );

    /// @brief Draw a sprite from atlas coordinates.
    /// @param srcX Source X in texture (pixels)
    /// @param srcY Source Y in texture (pixels)
    /// @param srcW Source width (pixels)
    /// @param srcH Source height (pixels)
    /// @param dstX Destination X (world/screen coordinates)
    /// @param dstY Destination Y (world/screen coordinates)
    /// @param dstW Destination width
    /// @param dstH Destination height
    /// @param r Red tint (0-255)
    /// @param g Green tint (0-255)
    /// @param b Blue tint (0-255)
    /// @param a Alpha (0-255)
    void draw(
      float srcX, float srcY, float srcW, float srcH,
      float dstX, float dstY, float dstW, float dstH,
      uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255
    );

    /// @brief Draw a sprite using normalized texture coordinates.
    void drawUV(
      float srcX, float srcY, float srcW, float srcH,
      float dstX, float dstY, float dstW, float dstH,
      uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255
    );

    /// @brief End recording and return vertex count.
    /// @return Number of vertices written (for vertex pool endFrame)
    size_t end();

    /// @brief Issue the draw call during render pass.
    /// @param pass Active render pass
    /// @param pipeline Pipeline to use
    /// @param vertex_buffer GPU vertex buffer (from vertex pool)
    void render(
      SDL_GPURenderPass* pass,
      SDL_GPUGraphicsPipeline* pipeline,
      SDL_GPUBuffer* vertex_buffer
    );

    /// @brief Get current sprite count.
    size_t getSpriteCount() const { return sprite_count_; }

    /// @brief Get current vertex count.
    size_t getVertexCount() const { return vertex_count_; }

    /// @brief Get bound texture.
    SDL_GPUTexture* getTexture() const { return texture_; }

    /// @brief Get bound sampler.
    SDL_GPUSampler* getSampler() const { return sampler_; }

    /// @brief Check if batch has any sprites.
    bool hasSprites() const { return sprite_count_ > 0; }

    /// @brief Get the index buffer for rendering.
    SDL_GPUBuffer* getIndexBuffer() const { return index_buffer_.get(); }

  private:
    void addQuad(
      float x0, float y0, float x1, float y1,
      float u0, float v0, float u1, float v1,
      uint8_t r, uint8_t g, uint8_t b, uint8_t a
    );

    SDL_GPUDevice* device_{ nullptr };
    SDL_GPUTexture* texture_{ nullptr };
    SDL_GPUSampler* sampler_{ nullptr };

    GPUBuffer index_buffer_;

    SpriteVertex* write_ptr_{ nullptr };
    size_t max_vertices_{ 0 };

    float texture_width_{ 1.0f };
    float texture_height_{ 1.0f };

    size_t sprite_count_{ 0 };
    size_t vertex_count_{ 0 };
    bool recording_{ false };
    bool initialized_{ false };
  };

} // namespace Simulacrum
