#include <spdlog/spdlog.h>
#include "UIManager.hpp"
#include "UIConstants.hpp"
#include "SimulacrumEngine.hpp"
#include "InputManager.hpp"
#include "GpuRenderer.hpp"
#include "GpuVertexPool.hpp"
#include "GpuTypes.hpp"
#include "SpriteBatch.hpp"
#include "TextureManager.hpp"

#include <algorithm>
#include <format>
#include <numeric>

namespace Simulacrum
{

  bool UIManager::init()
  {
    if (is_shutdown_)
    {
      return false;
    }

    const auto& engine = SimulacrumEngine::Instance();

    gpu_primitive_commands.reserve(GPU_PRIMITIVE_COMMAND_CAPACITY);

    return true;
  }

  void UIManager::update(float delta_time)
  {

  }

  void UIManager::clean()
  {
    if (is_shutdown_)
    {
      return;
    }

    is_shutdown_ = true;
  }

  const std::vector<std::shared_ptr<UIComponent>>& UIManager::getSortedComponents() const
  {
    if (sorted_components_dirty_)
    {
      sorted_components_cache.clear();
      sorted_components_cache.reserve(components_.size());

      for (const auto& [id, component] : components_)
      {
        if (component)
        {
          sorted_components_cache.push_back(component);
        }
      }

      std::sort(
        sorted_components_cache.begin(),
        sorted_components_cache.end(),
        [](const std::shared_ptr<UIComponent>& a,
          const std::shared_ptr<UIComponent>& b)
        {
          return a->z_order < b->z_order;
        });

      sorted_components_dirty_ = false;
    }

    return sorted_components_cache;
  }

  void UIManager::createPanel(const std::string& id, const UIRect& bounds)
  {
    auto component = std::make_shared<UIComponent>();

    component->id = id;
    component->type = UIComponentType::PANEL;
    component->bounds = bounds;
    component->z_order = UIConstants::ZORDER_PANEL;

    components_[id] = component;
  }

  void UIManager::createImage(const std::string& id, const UIRect& bounds, const std::string& texture_id)
  {
    auto component = std::make_shared<UIComponent>();
    component->id = id;
    component->type = UIComponentType::IMAGE;
    component->bounds = scaleRect(bounds);
    component->texture_id = texture_id;
    component->z_order = UIConstants::ZORDER_IMAGE;

    components_[id] = component;
  }

  void UIManager::applyPositioning(std::shared_ptr<UIComponent> component, int width, int height)
  {
    if (!component) return;

    auto& pos = component->positioning;
    auto& bounds = component->bounds;

    if (pos.width_percentage > 0.0f && pos.width_percentage <= 1.0f)
    {
      bounds.width = static_cast<int>(width * pos.width_percentage);
    }
    else if (pos.fixed_width == -1)
    {
      bounds.width = width;
    }
    else if (pos.fixed_width < -1)
    {
      bounds.width = width + static_cast<int>(pos.fixed_width * global_scale_);
    }
    else if (pos.fixed_width > 0)
    {
      bounds.width = static_cast<int>(pos.fixed_width * global_scale_);
    }

    if (pos.height_percentage > 0.0f && pos.height_percentage <= 1.0f)
    {
      bounds.height = static_cast<int>(height * pos.height_percentage);
    }
    else if (pos.fixed_height == -1)
    {
      bounds.height = height;
    }
    else if (pos.fixed_height < -1)
    {
      bounds.height = height + static_cast<int>(pos.fixed_height * global_scale_);
    }
    else if (pos.fixed_height > 0)
    {
      bounds.height = static_cast<int>(pos.fixed_height * global_scale_);
    }

    int scaled_offset_x = static_cast<int>(pos.offset_x * global_scale_);
    int scaled_offset_y = static_cast<int>(pos.offset_y * global_scale_);

    switch (pos.mode)
    {
    case UIPositionMode::ABSOLUTE_:
      // No change - keep current position
      break;
    case UIPositionMode::CENTERED_H:
      // Horizontally centered + offset_x, fixed offset_y
      break;
    case UIPositionMode::CENTERED_V:
      break;
    case UIPositionMode::CENTERED_BOTH:
      break;
    case UIPositionMode::TOP_ALIGNED:
      break;
    case UIPositionMode::TOP_RIGHT:
      break;
    case UIPositionMode::BOTTOM_ALIGNED:
      break;
    case UIPositionMode::BOTTOM_CENTERED:
      break;
    case UIPositionMode::BOTTOM_RIGHT:
      break;
    case UIPositionMode::LEFT_ALIGNED:
      break;
    case UIPositionMode::RIGHT_ALIGNED:
      break;
    }
  }

  void UIManager::recordGPUVertices(GPURenderer& gpu_renderer)
  {
    gpu_primitive_commands.clear();
    gpu_image_commands.clear();
    gpu_text_commands.clear();

    auto& primitive_pool = gpu_renderer.getPrimitiveVertexPool();
    auto& ui_pool = gpu_renderer.getUIVertexPool();

    auto* primitive_base = static_cast<ColorVertex*>(primitive_pool.getMappedPtr());
    auto* ui_base = static_cast<SpriteVertex*>(ui_pool.getMappedPtr());

    if (!primitive_base || !ui_base)
    {
      return;
    }

    uint32_t primitive_offset = 0;
    uint32_t ui_offset = 0;

    auto addFilledRect = [&](const UIRect& rect, const SDL_Color& color)
      {
        if (primitive_offset + 6 > GPU_PRIMITIVE_VERTEX_LIMIT) return;

        float x = static_cast<float>(rect.x);
        float y = static_cast<float>(rect.y);
        float w = static_cast<float>(rect.width);
        float h = static_cast<float>(rect.height);

        ColorVertex* v = primitive_base + primitive_offset;

        // Triangle 1
        v[0] = { x, y, color.r, color.g, color.b, color.a };
        v[1] = { x + w, y, color.r, color.g, color.b, color.a };
        v[2] = { x + w, y + h, color.r, color.g, color.b, color.a };
        // Triangle 2
        v[3] = { x, y, color.r, color.g, color.b, color.a };
        v[4] = { x + w, y + h, color.r, color.g, color.b, color.a };
        v[5] = { x, y + h, color.r, color.g, color.b, color.a };

        UIGPUDrawCommand cmd;
        cmd.type = UIGPUDrawCommand::Type::Rect;
        cmd.vertex_offset = primitive_offset;
        cmd.vertex_count = 6;
        gpu_primitive_commands.push_back(cmd);
        primitive_offset += 6;
      };

    auto addTextureRect = [&](const std::string& texture_id, int x, int y, int w, int h)
      {
        if (ui_offset + 6 > GPU_UI_VERTEX_LIMIT) return;

        auto* texture_data = TextureManager::Instance().getGPUTextureData(texture_id);

        if (!texture_data || !texture_data->texture) return;

        float tex_width = texture_data->width;
        float tex_height = texture_data->height;

        float sx = static_cast<float>(x);
        float sy = static_cast<float>(y);
        float sw = static_cast<float>(w);
        float sh = static_cast<float>(h);

        SpriteVertex* v = ui_base + ui_offset;

        // Triangle 1
        v[0] = { sx,        sy,       0.0f, 0.0f,   255, 255, 255, 255 };
        v[1] = { sx + sw,   sy,       1.0f, 0.0f,   255, 255, 255, 255 };
        v[2] = { sx + sw,   sy + sh,  1.0f, 1.0f,   255, 255, 255, 255 };
        // Triangle 2
        v[3] = { sx,        sy,       0.0f, 0.0f,   255, 255, 255, 255 };
        v[4] = { sx + sw,   sy + sh,  1.0f, 1.0f,   255, 255, 255, 255 };
        v[5] = { sx,        sy + sh,  0.0f, 1.0f,   255, 255, 255, 255 };

        // // Top-left
        // v[0] = {sx, sy, 0.0f, 0.0f, 255, 255, 255, 255};
        // // Top-right
        // v[1] = {sx + sw, sy, 1.0f, 0.0f, 255, 255, 255, 255};
        // // Bottom-right
        // v[2] = {sx + sw, sy + sh, 1.0f, 1.0f, 255, 255, 255, 255};
        // // Bottom-left
        // v[3] = {sx, sy + sh, 0.0f, 1.0f, 255, 255, 255, 255};

        auto tex = (texture_data->texture->get());

        UIGPUDrawCommand cmd;
        cmd.type = UIGPUDrawCommand::Type::Image;
        cmd.texture = texture_data->texture->get();
        cmd.vertex_offset = ui_offset;
        cmd.vertex_count = 6;
        gpu_image_commands.push_back(cmd);
        ui_offset += 6;
      };

    // Render components in z-order
    const auto& sorted_components = getSortedComponents();
    for (const auto& component : sorted_components)
    {
      if (!component || !component->visible) continue;

      SDL_Color bg_color = component->style.background_color;

      switch (component->type)
      {
      case UIComponentType::PANEL:
        addFilledRect(component->bounds, bg_color);
        break;
      case UIComponentType::IMAGE:
        addTextureRect(
          component->texture_id,
          component->bounds.x,
          component->bounds.y,
          component->bounds.width,
          component->bounds.height
        );
        break;
      }
    }

    primitive_pool.setWrittenVertexCount(primitive_offset);
    ui_pool.setWrittenVertexCount(ui_offset);
  }

  void UIManager::renderGPU(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass)
  {
    if (!swapchain_pass || gpu_image_commands.empty()) return;

    float ortho_matrix[16];
    GPURenderer::createOrthoMatrix(
      0.0f, static_cast<float>(gpu_renderer.getViewportWidth()),
      static_cast<float>(gpu_renderer.getViewportHeight()), 0.0f,
      ortho_matrix
    );

    // Render primitives (filled rectangles)
    if (!gpu_primitive_commands.empty())
    {
      SDL_BindGPUGraphicsPipeline(swapchain_pass, gpu_renderer.getUIPrimitivePipeline());
      gpu_renderer.pushViewProjection(swapchain_pass, ortho_matrix);

      SDL_GPUBufferBinding vertex_binding{};
      vertex_binding.buffer = gpu_renderer.getPrimitiveVertexPool().getGPUBuffer();
      vertex_binding.offset = 0;
      SDL_BindGPUVertexBuffers(swapchain_pass, 0, &vertex_binding, 1);

      uint32_t total_vertices = std::accumulate(
        gpu_primitive_commands.begin(), gpu_primitive_commands.end(), 0u,
        [](uint32_t sum, const auto& cmd)
        {
          return sum + cmd.vertex_count;
        }
      );

      if (total_vertices > 0)
      {
        SDL_DrawGPUPrimitives(swapchain_pass, total_vertices, 1, 0, 0);
      }
    }

    // Render images
    if (!gpu_image_commands.empty())
    {
      SDL_BindGPUGraphicsPipeline(swapchain_pass, gpu_renderer.getUISpritePipeline());
      gpu_renderer.pushViewProjection(swapchain_pass, ortho_matrix);

      // Bind vertex buffer
      SDL_GPUBufferBinding vertex_binding{};
      vertex_binding.buffer = gpu_renderer.getSpriteVertexPool().getGPUBuffer();
      vertex_binding.offset = 0;
      SDL_BindGPUVertexBuffers(swapchain_pass, 0, &vertex_binding, 1);

      // Bind index buffer
      auto& batch = gpu_renderer.getSpriteBatch();
      SDL_GPUBufferBinding index_binding{};
      index_binding.buffer = batch.getIndexBuffer();
      index_binding.offset = 0;
      SDL_BindGPUIndexBuffer(swapchain_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

      // Draw each texture
      for (const auto& cmd : gpu_image_commands)
      {
        SDL_GPUTextureSamplerBinding tex_sampler{};
        tex_sampler.texture = cmd.texture;
        tex_sampler.sampler = gpu_renderer.getLinearSampler();
        SDL_BindGPUFragmentSamplers(swapchain_pass, 0, &tex_sampler, 1);

        uint32_t total_vertices = std::accumulate(
          gpu_image_commands.begin(), gpu_image_commands.end(), 0u,
          [](uint32_t sum, const auto& cmd)
          {
            return sum + cmd.vertex_count;
          }
        );

        if (total_vertices > 0)
        {
          SDL_DrawGPUPrimitives(swapchain_pass, total_vertices, 1, 0, 0);
        }

        // Draw this sprite (6 indices per quad)
        // first_index = (vertex_offset / 4) * 6 because index buffer has 6 indices per 4 vertices
        // uint32_t first_index = (cmd.vertex_offset / 4) * 6;
        // SDL_DrawGPUIndexedPrimitives(swapchain_pass, 6, 1, first_index, 0, 0);
      }
    }
  }

} // namespace Simulacrum
