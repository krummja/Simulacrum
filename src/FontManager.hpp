#ifndef SIMULACRUM_FONT_MANAGER_HPP_
#define SIMULACRUM_FONT_MANAGER_HPP_

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include "GpuTexture.hpp"
#include "GpuRenderer.hpp"

namespace Simulacrum
{
  class GPURenderer;

  struct GPUTextData
  {
    std::unique_ptr<GPUTexture> texture;
    int width{ 0 };
    int height{ 0 };
  };

  class FontManager
  {
  public:
    static FontManager& Instance()
    {
      static FontManager instance;
      return instance;
    }

    bool init();

    bool loadFont(const std::string& font_file, const std::string& font_id, int font_size);

    bool loadFontsForDisplay(const std::string& font_path, int window_width, int window_height, float dpi_scape = 1.0f);

    bool isFontLoaded(const std::string& font_id) const;

    bool isShutdown() const { return is_shutdown_; }

    bool areFontsLoaded() const { return fonts_loaded_.load(std::memory_order_acquire); }

    bool measureTextWithWrapping(const std::string& text, const std::string& font_id, int max_width, int* width, int* height);

    std::vector<std::string> wrapTextToLines(const std::string& text, const std::string& font_id, int max_width);

    bool reloadFontsForDisplay(const std::string& font_path, int window_width, int window_height, float dpi_scale = 1.0f);

    void clean();

    bool measureText(const std::string& text, const std::string& font_id, int* width, int* height);

    bool getFontMetrics(const std::string& font_id, int* line_height, int* ascent, int* descent);

    bool measureMultilineText(const std::string& text, const std::string& font_id, int max_width, int* width, int* height);

    void clearFont(const std::string& font_id);

    const GPUTextData* renderTextGPU(const std::string& text, const std::string& font_id, SDL_Color color);

    void drawTextGPU(
      const std::string& text,
      const std::string& font_id,
      int x, int y, SDL_Color color,
      GPURenderer& gpu_renderer,
      SDL_GPURenderPass* render_pass
    );

    void clearGPUTextCache();

  private:
    /// @brief Cache for rendered text textures to avoid recreation
    struct TextCacheKey
    {
      std::string text;
      std::string font_id;
      SDL_Color color;

      bool operator==(const TextCacheKey& other) const
      {
        return text == other.text &&
          font_id == other.font_id &&
          color.r == other.color.r &&
          color.g == other.color.g &&
          color.b == other.color.b &&
          color.a == other.color.a;
      }
    };

    struct TextCacheKeyHash
    {
      std::size_t operator()(const TextCacheKey& key) const
      {
        return std::hash<std::string>()(key.text) ^
          std::hash<std::string>()(key.font_id) ^
          (static_cast<std::size_t>(key.color.r) << 24 |
           static_cast<std::size_t>(key.color.g) << 16 |
           static_cast<std::size_t>(key.color.b) << 8 |
           static_cast<std::size_t>(key.color.a));
      }
    };

    std::unordered_map<std::string, std::shared_ptr<TTF_Font>> font_map_{};
    std::unordered_map<TextCacheKey, std::unique_ptr<GPUTextData>, TextCacheKeyHash> text_cache_{};
    // std::unordered_map<TextCacheKey, std::shared_ptr<SDL_Texture>, TextCacheKeyHash> text_cache_{};
    std::atomic<bool> fonts_loaded_{ false };
    bool is_shutdown_{ false };
    std::vector<std::string> font_file_paths_{};
    std::mutex font_load_mutex_{};

    int last_window_width_{ 0 };
    int last_window_height_{ 0 };
    std::string last_font_path_{};

    // No copy and no assignment
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    FontManager() = default;
  };
}

#endif