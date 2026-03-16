#include "TextureManager.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <filesystem>
#include <format>
#include <cstring>

#include "GpuDevice.hpp"
#include "GpuTexture.hpp"
#include "GpuTransferBuffer.hpp"

namespace Simulacrum
{

  void TextureManager::clearFromTextureMap(const std::string& texture_id)
  {
    spdlog::info("Cleared : {} texture", texture_id);
    texture_map_.erase(texture_id);
  }

  bool TextureManager::isTextureInMap(const std::string& texture_id) const
  {
    return texture_map_.find(texture_id) != texture_map_.end();
  }

  void TextureManager::clean()
  {
    if (is_shutdown_) return;

    {
      std::lock_guard<std::mutex> lock(gpu_texture_mutex_);
      texture_map_.clear();
      pending_uploads_.clear();
    }

    is_shutdown_ = true;
  }

  bool TextureManager::loadGPU(const std::string& file_name, const std::string& texture_id)
  {
    std::lock_guard<std::mutex> lock(gpu_texture_mutex_);

    // Check if already loaded
    if (texture_map_.find(texture_id) != texture_map_.end())
    {
      spdlog::info("GPU texture already loaded: {}", texture_id);
      return true;
    }

    // Check if it's a directory
    if (std::filesystem::exists(file_name) && std::filesystem::is_directory(file_name))
    {
      spdlog::info("Loading GPU textures from directory: {}", file_name);

      bool loaded_any = false;
      int textures_loaded = 0;

      try
      {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(file_name))
        {
          if (!entry.is_regular_file())
          {
            spdlog::warn("Encountered non-regular file: {}", file_name);
            continue;
          }

          const auto& file_path = entry.path();
          std::string extension = file_path.extension().string();
          std::transform(
            extension.begin(), extension.end(), extension.begin(),
            [](unsigned char c) { return std::tolower(c); }
          );

          if (extension == ".png")
          {
            std::string full_path = file_path.string();
            std::string filename = file_path.stem().string();
            std::string combined_id = texture_id.empty() ? filename : std::format("{}_{}", texture_id, file_name);

            if (loadSingleGPUTexture(full_path, combined_id))
            {
              loaded_any = true;
              textures_loaded++;
            }
          }
        }
      }

      catch (const std::filesystem::filesystem_error& exc)
      {
        spdlog::error("GPU texture filesystem error: {}", exc.what());
      }

      catch (const std::exception& exc)
      {
        spdlog::error("GPU texture load error: {}", exc.what());
      }

      spdlog::info("Loaded {} GPU textures from directory: {}", textures_loaded, file_name);
      return loaded_any;
    }

    return loadSingleGPUTexture(file_name, texture_id);
  }

  bool TextureManager::loadSingleGPUTexture(const std::string& file_name, const std::string& texture_id)
  {
    SDL_Surface* raw_surface = SDL_LoadPNG(file_name.c_str());

    if (!raw_surface)
    {
      spdlog::error("GPU texture: could not load image: {} - {}", file_name, SDL_GetError());
      return false;
    }

    // Convert to ABRG8888 for GPU upload (maps to R8G8B8A8_UNORM byte order on little-indian)
    // SDL_PIXELFORMAT_ARGB8888: 32-bit with A in high bits -> memory loadout R,G,B,A on LE
    SDL_Surface* converted_surface = raw_surface;
    if (raw_surface->format != SDL_PIXELFORMAT_ARGB8888)
    {
      converted_surface = SDL_ConvertSurface(raw_surface, SDL_PIXELFORMAT_ARGB8888);
      SDL_DestroySurface(raw_surface);
      if (!converted_surface)
      {
        spdlog::error("GPU texture: could not convert surface: {}", SDL_GetError());
        return false;
      }
    }

    SDL_PremultiplyAlpha(
      converted_surface->w, converted_surface->h,
      SDL_PIXELFORMAT_ARGB8888, converted_surface->pixels, converted_surface->pitch,
      SDL_PIXELFORMAT_ARGB8888, converted_surface->pixels, converted_surface->pitch,
      false
    );

    auto& gpu_device = GPUDevice::Instance();
    if (!gpu_device.isInitialized())
    {
      spdlog::error("GPU texture: GPUDevice not initialized");
      SDL_DestroySurface(converted_surface);
      return false;
    }

    uint32_t width = static_cast<uint32_t>(converted_surface->w);
    uint32_t height = static_cast<uint32_t>(converted_surface->h);

    auto gpu_texture = std::make_unique<GPUTexture>(
      gpu_device.get(),
      width, height,
      SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,
      SDL_GPU_TEXTUREUSAGE_SAMPLER
    );

    if (!gpu_texture->isValid())
    {
      spdlog::error("GPU texture: failed to create texture for: {}", texture_id);
      SDL_DestroySurface(converted_surface);
      return false;
    }

    TextureData data;
    data.texture = std::move(gpu_texture);
    data.width = static_cast<float>(width);
    data.height = static_cast<float>(height);
    texture_map_[texture_id] = std::move(data);

    pending_uploads_.push_back(PendingTextureUpload{
      texture_id,
      std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>(converted_surface, SDL_DestroySurface),
      width,
      height
      });

    // spdlog::debug("GPU texture queued for upload: {} ({}x{})", texture_id, width, height);
    return true;
  }

  void TextureManager::processPendingUploads(SDL_GPUCopyPass* copy_pass)
  {
    if (!copy_pass || pending_uploads_.empty()) return;

    std::lock_guard<std::mutex> lock(gpu_texture_mutex_);

    auto& gpu_device = GPUDevice::Instance();
    if (!gpu_device.isInitialized())
    {
      spdlog::error("GPU texture upload: GPUDevice not initialized");
      return;
    }

    for (auto& pending : pending_uploads_)
    {
      auto it = texture_map_.find(pending.texture_id);
      if (it == texture_map_.end())
      {
        spdlog::warn("GPU texture upload: null surface for: {}", pending.texture_id);
        continue;
      }

      SDL_Surface* surface = pending.surface.get();
      if (!surface)
      {
        spdlog::warn("GPU texture upload: null surface for: {}", pending.texture_id);
        continue;
      }

      uint32_t data_size = static_cast<uint32_t>(surface->pitch * surface->h);

      GPUTransferBuffer transfer_buffer{
        gpu_device.get(),
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        data_size
      };

      if (!transfer_buffer.isValid())
      {
        spdlog::error("GPU texture upload: failed to create transfer buffer for: {}", pending.texture_id);
        continue;
      }

      void* mapped = transfer_buffer.map(false);
      if (!mapped)
      {
        spdlog::error("GPU texture upload: failed to map transfer buffer for: {}", pending.texture_id);
        continue;
      }

      std::memcpy(mapped, surface->pixels, data_size);
      transfer_buffer.unmap();

      SDL_GPUTextureTransferInfo src_info{};
      src_info.transfer_buffer = transfer_buffer.get();
      src_info.offset = 0;
      src_info.pixels_per_row = pending.width;
      src_info.rows_per_layer = pending.height;

      SDL_GPUTextureRegion dst_region{};
      dst_region.texture = it->second.texture->get();
      dst_region.x = 0;
      dst_region.y = 0;
      dst_region.z = 0;
      dst_region.w = pending.width;
      dst_region.h = pending.height;
      dst_region.d = 1;

      SDL_UploadToGPUTexture(copy_pass, &src_info, &dst_region, false);
      // spdlog::debug("GPU texture uploaded: {} ({}x{})", pending.texture_id, pending.width, pending.height);
    }

    pending_uploads_.clear();
  }

  GPUTexture* TextureManager::getGPUTexture(const std::string& texture_id) const
  {
    std::lock_guard<std::mutex> lock(gpu_texture_mutex_);

    auto it = texture_map_.find(texture_id);
    if (it != texture_map_.end())
    {
      return it->second.texture.get();
    }

    spdlog::warn("No GPU texture found with id: {}", texture_id);
    return nullptr;
  }

  const TextureData* TextureManager::getGPUTextureData(const std::string& texture_id) const
  {
    std::lock_guard<std::mutex> lock(gpu_texture_mutex_);

    auto it = texture_map_.find(texture_id);
    if (it != texture_map_.end())
    {
      return &it->second;
    }

    spdlog::warn("No texture data for id: {} - returning null pointer", texture_id);
    return nullptr;
  }

} // namespace Simulacrum
