#include "FontManager.hpp"
#include "GpuDevice.hpp"
#include "GpuTransferBuffer.hpp"
#include "GpuTypes.hpp"
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cmath>
#include <cstring>
#include <sstream>
#include <format>
#include <spdlog/spdlog.h>

namespace
{
  constexpr float HEIGHT_RATIO = 90.0f;

  constexpr float UI_FONT_RATIO = 0.875f;
  constexpr float TITLE_FONT_RATIO = 1.5f;
  constexpr float TOOLTIP_FONT_RATIO = 0.6f;

  constexpr int MAX_FONT_SIZE = 100;

  constexpr int MIN_BASE_FONT_SIZE = 18;
  constexpr int MIN_UI_FONT_SIZE = 16;
  constexpr int MIN_TITLE_FONT_SIZE = 24;
  constexpr int MIN_TOOLTIP_FONT_SIZE = 12;
}

namespace Simulacrum
{
  bool FontManager::init()
  {
    if (!TTF_Init())
    {
      spdlog::critical("Font system initialization failed: {}", SDL_GetError());
      return false;
    }

    is_shutdown_ = false;
    spdlog::info("Font system initialized with quality hints");
    return true;
  }

  bool FontManager::loadFontsForDisplay(const std::string& font_path, int window_width, int window_height, float dpi_scale)
  {
    if (fonts_loaded_.load(std::memory_order_acquire))
    {
      return true;
    }

    std::lock_guard<std::mutex> lock(font_load_mutex_);

    if (fonts_loaded_.load(std::memory_order_acquire))
    {
      return true;
    }

    // Scan directory for font files if not already done
    if (font_file_paths_.empty())
    {
      try
      {
        for (const auto& entry : std::filesystem::directory_iterator(font_path))
        {
          if (entry.is_regular_file())
          {
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
              [](unsigned char c) { return std::tolower(c); }
            );

            if (extension == ".ttf" || extension == ".otf")
            {
              font_file_paths_.push_back(entry.path().string());
            }
          }
        }

        // Sort to ensure consistent loading order
        std::sort(font_file_paths_.begin(), font_file_paths_.end());
      }

      catch (const std::filesystem::filesystem_error& exc)
      {
        spdlog::error("Filesystem error while scanning for fonts: {}", exc.what());
        return false;
      }
    }

    // Ensure dpi scale is valid
    float const effective_dpi_scale = (dpi_scale > 0.0f) ? dpi_scale : 1.0f;

    // Calculate font sizes based on logical window height first
    // Apply minimums for readability, then scale by DPI for high-density displays
    int clamped_height = std::clamp(window_height, 480, 8640);
    float const logical_base_size_float = static_cast<float>(clamped_height) / HEIGHT_RATIO;

    // Apply minimums at logical scale, then multiply by DPI scale for pixel rendering
    int logical_base = std::max(static_cast<int>(std::round(logical_base_size_float)), MIN_BASE_FONT_SIZE);
    int logical_ui = std::max(static_cast<int>(std::round(logical_base_size_float * UI_FONT_RATIO)), MIN_UI_FONT_SIZE);
    int logical_title = std::max(static_cast<int>(std::round(logical_base_size_float * TITLE_FONT_RATIO)), MIN_TITLE_FONT_SIZE);;
    int logical_tooltip = std::max(static_cast<int>(std::round(logical_base_size_float * TOOLTIP_FONT_RATIO)), MIN_TOOLTIP_FONT_SIZE);;

    // Scale by DPI for pixel-perfect rendering on high-density displays
    int base_font_size = std::min(static_cast<int>(std::round(logical_base * effective_dpi_scale)), MAX_FONT_SIZE);
    int ui_font_size = std::min(static_cast<int>(std::round(logical_ui * effective_dpi_scale)), MAX_FONT_SIZE);;
    int title_font_size = std::min(static_cast<int>(std::round(logical_title * effective_dpi_scale)), MAX_FONT_SIZE);;
    int tooltip_font_size = std::min(static_cast<int>(std::round(logical_tooltip * effective_dpi_scale)), MAX_FONT_SIZE);;

    spdlog::info("Calculated font sizes (dpi_scale={}, logical={}): base={}, UI={}, title={}, tooltip={}",
      effective_dpi_scale, logical_base, base_font_size, ui_font_size, title_font_size, tooltip_font_size
    );

    bool success = true;
    for (const auto& file_path : font_file_paths_)
    {
      std::string filename = std::filesystem::path(file_path).stem().string();
      success &= loadFont(file_path, std::format("fonts_{}", filename), base_font_size);
      success &= loadFont(file_path, std::format("fonts_UI_{}", filename), ui_font_size);
      success &= loadFont(file_path, std::format("fonts_title_{}", filename), title_font_size);
      success &= loadFont(file_path, std::format("fonts_tooltip_{}", filename), tooltip_font_size);
    }

    if (success)
    {
      last_window_width_ = window_width;
      last_window_height_ = window_height;
      last_font_path_ = font_path;
      fonts_loaded_.store(true, std::memory_order_release);
      spdlog::info("All font templates loaded successfully");
    }

    return success;
  }

  bool FontManager::loadFont(const std::string& font_file, const std::string& font_id, int font_size)
  {
    auto font = std::shared_ptr<TTF_Font>(TTF_OpenFont(font_file.c_str(), font_size), TTF_CloseFont);

    if (!font)
    {
      spdlog::error("Failed to load font '{}' with size {}: {}", font_file, font_size, SDL_GetError());
      return false;
    }

    TTF_SetFontHinting(font.get(), TTF_HINTING_NORMAL);
    TTF_SetFontKerning(font.get(), 1);
    TTF_SetFontStyle(font.get(), TTF_STYLE_NORMAL);

    font_map_[font_id] = std::move(font);
    spdlog::info("Loaded font '{}' from '{}'", font_id, font_file);
    return true;
  }

  std::vector<std::string> FontManager::wrapTextToLines(const std::string& text, const std::string& font_id, int max_width)
  {
    std::vector<std::string> wrapped_lines;

    if (is_shutdown_ || max_width <= 0)
    {
      // Return original text if invalid params
      wrapped_lines.push_back(text);
      return wrapped_lines;
    }

    auto font_it = font_map_.find(font_id);
    if (font_it == font_map_.end())
    {
      spdlog::error("Font '{}' not found for text wrapping", font_id);
      wrapped_lines.push_back(text);
      return wrapped_lines;
    }

    // First split by explicit newlines
    std::vector<std::string> lines;
    std::string current_line;
    for (char c : text)
    {
      if (c == '\n')
      {
        lines.push_back(current_line);
        current_line.clear();
      }
      else
      {
        current_line += c;
      }
    }

    if (!current_line.empty())
    {
      lines.push_back(current_line);
    }

    // Now wrap each line if it's too wide
    for (const auto& line : lines)
    {
      if (line.empty())
      {
        wrapped_lines.push_back("");
        continue;
      }

      std::string working_line;
      std::istringstream words(line);
      std::string word;

      while (words >> word)
      {
        std::string test_line = working_line;
        if (!test_line.empty())
        {
          test_line += " ";
        }
        test_line += word;
        int test_width = 0;

        if (TTF_GetStringSize(font_it->second.get(), test_line.c_str(), 0, &test_width, nullptr))
        {
          if (test_width <= max_width)
          {
            working_line = test_line;
          }

          else
          {
            if (!working_line.empty())
            {
              wrapped_lines.push_back(working_line);
              working_line = word;
            }

            else
            {
              // Single word is too long - just add it
              wrapped_lines.push_back(word);
              working_line.clear();
            }
          }
        }

        else
        {
          // If measurement fails, just add the word
          working_line = test_line;
        }
      }

      if (!working_line.empty())
      {
        wrapped_lines.push_back(working_line);
      }
    }

    return wrapped_lines;
  }

  bool FontManager::isFontLoaded(const std::string& font_id) const
  {
    return font_map_.find(font_id) != font_map_.end();
  }

  void FontManager::clearFont(const std::string& font_id)
  {
    if (font_map_.erase(font_id) > 0)
    {
      spdlog::info("Cleared font: {}", font_id);
    }
  }

  bool FontManager::reloadFontsForDisplay(const std::string& font_path, int window_width, int window_height, float dpi_scale)
  {
    if (is_shutdown_)
    {
      spdlog::warn("Cannot reload fonts - FontManager is shut down");
      return false;
    }

    font_map_.clear();
    text_cache_.clear();

    fonts_loaded_.store(false, std::memory_order_release);

    // Reset display tracking
    last_window_width_ = 0;
    last_window_height_ = 0;
    last_font_path_.clear();

    return loadFontsForDisplay(font_path, window_width, window_height, dpi_scale);
  }

  void FontManager::clean()
  {
    if (is_shutdown_)
    {
      return;
    }

    [[maybe_unused]] int fonts_freed = font_map_.size();
    is_shutdown_ = true;

    font_map_.clear();
    text_cache_.clear();

    last_window_width_ = 0;
    last_window_height_ = 0;
    last_font_path_.clear();

    spdlog::info("{} fonts freed", fonts_freed);
    spdlog::info("FontManager resources cleaned - TTF will be cleaned by SDL_Quit()");
  }

  bool FontManager::measureTextWithWrapping(const std::string& text, const std::string& font_id, int max_width, int* width, int* height)
  {
    if (is_shutdown_ || !width || !height)
    {
      return false;
    }

    auto font_it = font_map_.find(font_id);
    if (font_it == font_map_.end())
    {
      spdlog::error("Font '{}' not found for wrapped measurement", font_id);
      return false;
    }

    auto wrapped_lines = wrapTextToLines(text, font_id, max_width);
    if (wrapped_lines.empty())
    {
      *width = 0;
      *height = 0;
      return true;
    }

    TTF_Font* font = font_it->second.get();
    int const line_height = TTF_GetFontHeight(font);
    int max_line_width = 0;

    // Measure each wrapped line to get the actual maximum width
    for (const auto& line : wrapped_lines)
    {
      int line_width = 0;
      if (!line.empty())
      {
        if (TTF_GetStringSize(font, line.c_str(), 0, &line_width, nullptr))
        {
          max_line_width = std::max(max_line_width, line_width);
        }
      }
    }

    *width = max_line_width;
    *height = line_height * static_cast<int>(wrapped_lines.size());
    return true;
  }

  bool FontManager::measureText(const std::string& text, const std::string& font_id, int* width, int* height)
  {
    if (is_shutdown_ || !width || !height)
    {
      return false;
    }

    auto font_it = font_map_.find(font_id);
    if (font_it == font_map_.end())
    {
      spdlog::error("Font '{}' not found for measurement", font_id);
      return false;
    }

    return TTF_GetStringSize(font_it->second.get(), text.c_str(), 0, width, height);
  }

  bool FontManager::measureMultilineText(const std::string& text, const std::string& font_id, int max_width, int* width, int* height)
  {
    if (is_shutdown_ || !width || !height){
      return false;
    }

    auto font_it = font_map_.find(font_id);
    if (font_it == font_map_.end())
    {
      spdlog::error("Font '{}' not found for multiline measurement", font_id);
      return false;
    }

    // Split text by newlines
    std::vector<std::string> lines;
    std::string current_line;
    for (char c : text)
    {
      if (c == '\n')
      {
        lines.push_back(current_line);
        current_line.clear();
      }
      else
      {
        current_line += c;
      }
    }

    if (!current_line.empty())
    {
      lines.push_back(current_line);
    }

    if (lines.empty())
    {
      *width = 0;
      *height = 0;
      return true;
    }

    TTF_Font* font = font_it ->second.get();
    int const line_height = TTF_GetFontHeight(font);
    int max_line_width = 0;

    // Measure each line
    for (const auto& line : lines)
    {
      int line_width = 0;
      if (!line.empty())
      {
        if (!TTF_GetStringSize(font, line.c_str(), 0, &line_width, nullptr))
        {
          spdlog::error("Failed to measure line: {}", line);
          return false;
        }
      }

      max_line_width = std::max(max_line_width, line_width);
    }

    // Apply max width constraint if specified
    if (max_width > 0 && max_line_width > max_width)
    {
      max_line_width = max_width;
    }

    *width = max_line_width;
    *height = line_height * static_cast<int>(lines.size());

    return true;
  }

  bool FontManager::getFontMetrics(const std::string& font_id, int* line_height, int* ascent, int* descent)
  {
    if (is_shutdown_ || !line_height || !ascent || !descent)
    {
      return false;
    }

    auto font_it = font_map_.find(font_id);
    if (font_it == font_map_.end())
    {
      spdlog::error("Font '{}' not found for metrics", font_id);
      return false;
    }

    TTF_Font* font = font_it->second.get();
    *line_height = TTF_GetFontHeight(font);
    *ascent = TTF_GetFontAscent(font);
    *descent = TTF_GetFontDescent(font);

    return true;
  }

  const GPUTextData* FontManager::renderTextGPU(const std::string& text, const std::string& font_id, SDL_Color color)
  {
    if (is_shutdown_ || text.empty())
    {
      return nullptr;
    }

    // Check cache first
    TextCacheKey key = { text, font_id, color };
    auto cache_it = text_cache_.find(key);
    if (cache_it != text_cache_.end())
    {
      return cache_it->second.get();
    }

    auto font_it = font_map_.find(font_id);
    if (font_it == font_map_.end())
    {
      spdlog::error("Font '{}' not found for rendering", font_id);
      return nullptr;
    }

    // Render text to surface
    SDL_Surface* surface = TTF_RenderText_Blended(font_it->second.get(), text.c_str(), 0, color);
    if (!surface)
    {
      spdlog::error("Failed to render text surface: {}", SDL_GetError());
      return nullptr;
    }

    // Convert to ABGR8888 for GPU upload (maps to R8G8B8A8_UNORM byte order on little-endian)
    SDL_Surface* converted = nullptr;
    if (surface->format != SDL_PIXELFORMAT_ABGR8888)
    {
      converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);
      SDL_DestroySurface(surface);

      if (!converted)
      {
        spdlog::error("Failed to convert text surface: {}", SDL_GetError());
        return nullptr;
      }

      surface = converted;
    }

    // Premultiply alpha for proper blending
    SDL_PremultiplyAlpha(
      surface->w, surface->h,
      SDL_PIXELFORMAT_ABGR8888, surface->pixels, surface->pitch,
      SDL_PIXELFORMAT_ABGR8888, surface->pixels, surface->pitch,
      false
    );

    // Create GPU texture data
    auto gpu_data = std::make_unique<GPUTextData>();
    gpu_data->width = surface->w;
    gpu_data->height = surface->h;

    auto& gpu_device = GPUDevice::Instance();
    SDL_GPUDevice* device = gpu_device.get();

    // Create GPU texture
    gpu_data->texture = std::make_unique<GPUTexture>(
      device,
      static_cast<uint32_t>(surface->w),
      static_cast<uint32_t>(surface->h),
      SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
      SDL_GPU_TEXTUREUSAGE_SAMPLER
    );

    if (!gpu_data->texture->isValid())
    {
      spdlog::error("Failed to create GPU texture for text");
      SDL_DestroySurface(surface);
      return nullptr;
    }

    // Upload texture data immediately using a transfer buffer
    uint32_t data_size = static_cast<uint32_t>(surface->pitch * surface->h);
    GPUTransferBuffer transfer_buffer(device, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, data_size);

    if (!transfer_buffer.isValid())
    {
      spdlog::error("Failed to create transfer buffer for text upload");
      SDL_DestroySurface(surface);
      return nullptr;
    }

    // Copy pixel data to transfer buffer
    void* mapped = transfer_buffer.map(false);
    if (mapped)
    {
      std::memcpy(mapped, surface->pixels, data_size);
      transfer_buffer.unmap();
    }

    // Upload using one-off command buffer
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (command_buffer)
    {
      SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
      if (copy_pass)
      {
        SDL_GPUTextureTransferInfo src{};
        src.transfer_buffer = transfer_buffer.get();
        src.offset = 0;
        src.pixels_per_row = static_cast<uint32_t>(surface->w);
        src.rows_per_layer = static_cast<uint32_t>(surface->h);

        SDL_GPUTextureRegion dst{};
        dst.texture = gpu_data->texture->get();
        dst.w = static_cast<uint32_t>(surface->w);
        dst.h = static_cast<uint32_t>(surface->h);
        dst.d = 1;

        SDL_UploadToGPUTexture(copy_pass, &src, &dst, false);
        SDL_EndGPUCopyPass(copy_pass);
      }

      SDL_SubmitGPUCommandBuffer(command_buffer);
    }

    SDL_DestroySurface(surface);

    // Cache and return
    GPUTextData* result = gpu_data.get();
    text_cache_[key] = std::move(gpu_data);
    return result;
  }

  void FontManager::drawTextGPU(
    const std::string& text, const std::string& font_id,
    int x, int y, SDL_Color color,
    GPURenderer& gpu_renderer,
    SDL_GPURenderPass* pass
  )
  {
    if (!pass || text.empty())
    {
      return;
    }

    const GPUTextData* text_data = renderTextGPU(text, font_id, color);
    if (!text_data || !text_data->texture || !text_data->texture->isValid())
    {
      return;
    }

    // Calculate centered position
    float dst_x = static_cast<float>(x - text_data->width / 2);
    float dst_y = static_cast<float>(y - text_data->height / 2);
    float dst_w = static_cast<float>(text_data->width);
    float dst_h = static_cast<float>(text_data->height);

    float ortho_matrix[16];
    GPURenderer::createOrthoMatrix(
      0.0f, static_cast<float>(gpu_renderer.getViewportWidth()),
      static_cast<float>(gpu_renderer.getViewportHeight()), 0.0f,
      ortho_matrix
    );

    // Bind UI sprite pipeline (for swapchain rendering)
    SDL_BindGPUGraphicsPipeline(pass, gpu_renderer.getUISpritePipeline());

    // Push view projection
    gpu_renderer.pushViewProjection(pass, ortho_matrix);

    // Create quad vertices
    SpriteVertex vertices[4];
    vertices[0] = { dst_x,          dst_y,          0.0f, 0.0f,   255, 255, 255, 255 };
    vertices[1] = { dst_x + dst_w,  dst_y,          1.0f, 0.0f,   255, 255, 255, 255 };
    vertices[2] = { dst_x + dst_w,  dst_y + dst_h,  1.0f, 1.0f,   255, 255, 255, 255 };
    vertices[3] = { dst_x,          dst_y + dst_h,  0.0f, 1.0f,   255, 255, 255, 255 };

    // Use the sprite batch's index buffer for the quad
    auto& batch = gpu_renderer.getSpriteBatch();

    // Bind texture
    SDL_GPUTextureSamplerBinding tex_sampler{};
    tex_sampler.texture = text_data->texture->get();
    tex_sampler.sampler = gpu_renderer.getLinearSampler();
    SDL_BindGPUFragmentSamplers(pass, 0, &tex_sampler, 1);

    // For text, we use immediate drawing with the sprite batch index buffer
    // Bind the vertex buffer from sprite pool (it has our vertices)
    SDL_GPUBufferBinding vertex_binding{};
    vertex_binding.buffer = gpu_renderer.getSpriteVertexPool().getGPUBuffer();
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);

    // Bind the index buffer
    SDL_GPUBufferBinding index_binding{};
    index_binding.buffer = batch.getIndexBuffer();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    // Draw the quad
    SDL_DrawGPUIndexedPrimitives(pass, 6, 1, 0, 0, 0);
  }

  void FontManager::clearGPUTextCache()
  {
    text_cache_.clear();
    spdlog::debug("GPU text cache cleared");
  }

}
