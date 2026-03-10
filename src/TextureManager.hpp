#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include "GpuTexture.hpp"

namespace Simulacrum
{

  struct TextureData
  {
    std::unique_ptr<GPUTexture> texture;
    float width{ 0.0f };
    float height{ 0.0f };
  };

  struct PendingTextureUpload
  {
    std::string texture_id;
    std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> surface;
    uint32_t width{ 0 };
    uint32_t height{ 0 };
  };

  class TextureManager
  {
  public:
    ~TextureManager()
    {
      if (!is_shutdown_)
      {
        clean();
      }
    }

    static TextureManager& Instance()
    {
      static TextureManager instance;
      return instance;
    }

    void clean();

    bool isShutdown() const { return is_shutdown_; }

    bool loadGPU(const std::string& file_name, const std::string& texture_id);

    void processPendingUploads(SDL_GPUCopyPass* copy_pass);

    GPUTexture* getGPUTexture(const std::string& texture_id) const;

    const TextureData* getGPUTextureData(const std::string& texture_id) const;

    bool hasGPUTextures() const { return !texture_map_.empty(); }

    bool hasPendingUploads() const { return !pending_uploads_.empty(); }

    void clearFromTextureMap(const std::string& texture_id);

    bool isTextureInMap(const std::string& texture_id) const;

  private:
    std::string texture_id_{ "" };
    std::unordered_map<std::string, TextureData> texture_map_{};
    std::vector<PendingTextureUpload> pending_uploads_{};
    std::atomic<bool> textures_loaded_{ false };
    std::mutex texture_load_mutex_{};
    bool is_shutdown_{ false };
    mutable std::mutex gpu_texture_mutex_{};

    bool loadSingleGPUTexture(const std::string& file_name, const std::string& texture_id);

    // Copy operations
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager() = default;
  };

} // namespace Simulacrum
