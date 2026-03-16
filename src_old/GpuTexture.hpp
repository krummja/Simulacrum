#pragma once

#include <SDL3/SDL_gpu.h>
#include <cstdint>

namespace Simulacrum
{

  /// @brief RAII wrapper for SDL_GPUTexture.
  /// Handles texture creation and provides helper methods
  /// for render target usage.
  class GPUTexture
  {
  public:
    GPUTexture() = default;

    /// @brief Create a 2D texture with specified parameters.
    /// @param device GPU device
    /// @param width Texture width in pixels
    /// @param height Texture height in pixels
    /// @param format Pixel format
    /// @param usage Usage flags (SAMPLER, COLOR_TARGET, etc.)
    /// @param num_levels Number of mip levels (1 for no mipmaps)
    GPUTexture(
      SDL_GPUDevice* device,
      uint32_t width,
      uint32_t height,
      SDL_GPUTextureFormat format,
      SDL_GPUTextureUsageFlags usage,
      uint32_t num_levels = 1
    );

    ~GPUTexture();

    // Move-only
    GPUTexture(GPUTexture&&) noexcept;
    GPUTexture& operator=(GPUTexture&&) noexcept;
    GPUTexture(const GPUTexture&) = delete;
    GPUTexture& operator=(const GPUTexture&) = delete;

    SDL_GPUTexture* get() const { return texture_; }
    uint32_t getWidth() const { return width_; }
    uint32_t getHeight() const { return height_; }
    SDL_GPUTextureFormat getFormat() const { return format_; }
    SDL_GPUTextureUsageFlags getUsage() const { return usage_; }
    bool isValid() const { return texture_ != nullptr; }

    bool isRenderTarget() const
    {
      return (usage_ & SDL_GPU_TEXTUREUSAGE_COLOR_TARGET) != 0;
    }

    bool isSampler() const
    {
      return (usage_ & SDL_GPU_TEXTUREUSAGE_SAMPLER) != 0;
    }

    /// @brief Create color target info for use in render passes
    /// @param load_op Load operation (CLEAR, LOAD, DONT_CARE)
    /// @param clear_color Clear color if load_op is CLEAR
    /// @param store_op Store operation (STORE< DONT_CARE)
    /// @return Configured color target info
    SDL_GPUColorTargetInfo asColorTarget(
      SDL_GPULoadOp load_op = SDL_GPU_LOADOP_CLEAR,
      SDL_FColor clear_color = { 0.0f, 0.0f, 0.0f, 0.0f },
      SDL_GPUStoreOp store_op = SDL_GPU_STOREOP_STORE
    ) const;

    /// @brief Create texture sampler binding for shader binding.
    SDL_GPUTextureSamplerBinding asSamplerBinding(SDL_GPUSampler* sampler) const;

  private:
    void release();

    SDL_GPUTexture* texture_{ nullptr };
    SDL_GPUDevice* device_{ nullptr };
    uint32_t width_{ 0 };
    uint32_t height_{ 0 };
    SDL_GPUTextureFormat format_{ SDL_GPU_TEXTUREFORMAT_INVALID };
    SDL_GPUTextureUsageFlags usage_{ 0 };
  };

} // namespace Simulacrum
