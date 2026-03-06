#include "gpu_texture.hpp"
#include <format>
#include <spdlog/spdlog.h>

namespace Simulacrum
{

  GPUTexture::GPUTexture(
    SDL_GPUDevice* device,
    uint32_t width,
    uint32_t height,
    SDL_GPUTextureFormat format,
    SDL_GPUTextureUsageFlags usage,
    uint32_t num_levels
  )
    : device_(device)
    , width_(width)
    , height_(height)
    , format_(format)
    , usage_(usage)
  {
    if (!device_)
    {
      spdlog::error("GPUTexture: null device");
      return;
    }

    if (width == 0 || height == 0)
    {
      spdlog::error("GPUTexture: invalid dimensions {}x{}", width, height);
      return;
    }

    SDL_GPUTextureCreateInfo create_info{};
    create_info.type = SDL_GPU_TEXTURETYPE_2D;
    create_info.format = format;
    create_info.usage = usage;
    create_info.width = width;
    create_info.height = height;
    create_info.layer_count_or_depth = 1;
    create_info.num_levels = num_levels;
    create_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    texture_ = SDL_CreateGPUTexture(device, &create_info);

    if (!texture_)
    {
      spdlog::error("Failed to create GPU texture {}x{}: {}", width, height, SDL_GetError());
    }
  }

  GPUTexture::~GPUTexture()
  {
    release();
  }

  GPUTexture::GPUTexture(GPUTexture&& other) noexcept
    : texture_(other.texture_)
    , device_(other.device_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
    , usage_(other.usage_)
  {
    other.texture_ = nullptr;
    other.device_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
  }

  GPUTexture& GPUTexture::operator=(GPUTexture&& other) noexcept
  {
    if (this != &other)
    {
      release();

      texture_ = other.texture_;
      device_ = other.device_;
      width_ = other.width_;
      height_ = other.height_;
      format_ = other.format_;
      usage_ = other.usage_;

      other.texture_ = nullptr;
      other.device_ = nullptr;
      other.width_ = 0;
      other.height_ = 0;
    }

    return *this;
  }

  void GPUTexture::release()
  {
    if (texture_ && device_)
    {
      SDL_ReleaseGPUTexture(device_, texture_);
      texture_ = nullptr;
    }
  }

  SDL_GPUColorTargetInfo GPUTexture::asColorTarget(
    SDL_GPULoadOp load_op,
    SDL_FColor clear_color,
    SDL_GPUStoreOp store_op
  ) const
  {
    SDL_GPUColorTargetInfo info{};
    info.texture = texture_;
    info.mip_level = 0;
    info.layer_or_depth_plane = 0;
    info.clear_color = clear_color;
    info.load_op = load_op;
    info.store_op = store_op;
    info.cycle = false;
    return info;
  }

  SDL_GPUTextureSamplerBinding GPUTexture::asSamplerBinding(SDL_GPUSampler* sampler) const
  {
    if (!texture_)
    {
      spdlog::warn("GPUTexture::asSamplerBinding() called on invalid texture");
    }

    if (!sampler)
    {
      spdlog::warn("GPUTexture::asSamplerBinding() called with null sampler");
    }

    SDL_GPUTextureSamplerBinding binding{};
    binding.texture = texture_;
    binding.sampler = sampler;
    return binding;
  }

} // namespace Simulacrum
