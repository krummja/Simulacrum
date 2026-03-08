#include <spdlog/spdlog.h>
#include "UIManager.hpp"
#include "UIConstants.hpp"
#include "SimulacrumEngine.hpp"
#include "InputManager.hpp"
#include "GpuRenderer.hpp"
#include "GpuVertexPool.hpp"
#include "GpuTypes.hpp"
#include "SpriteBatch.hpp"

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
      }
    }

    primitive_pool.setWrittenVertexCount(primitive_offset);
    ui_pool.setWrittenVertexCount(ui_offset);
  }

  void UIManager::renderGPU(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass)
  {
    if (!swapchain_pass) return;

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
  }

} // namespace Simulacrum
