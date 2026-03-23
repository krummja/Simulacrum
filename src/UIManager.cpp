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
#include "FontManager.hpp"

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

    setDarkTheme();

    global_font_id_ = UIConstants::FONT_DEFAULT;
    title_font_id_ = UIConstants::FONT_TITLE;
    ui_font_id_ = UIConstants::FONT_UI;

    components_.clear();
    layouts_.clear();

    const auto& engine = SimulacrumEngine::Instance();
    current_logical_width_ = engine.getLogicalWidth();
    current_logical_height_ = engine.getLogicalHeight();

    global_scale_ = calculateOptimalScale(current_logical_width_, current_logical_height_);

    gpu_primitive_commands.reserve(GPU_PRIMITIVE_COMMAND_CAPACITY);
    gpu_text_commands.reserve(GPU_TEXT_COMMAND_CAPACITY);
    gpu_image_commands.reserve(GPU_IMAGE_COMMAND_CAPACITY);

    return true;
  }

  void UIManager::update(float delta_time)
  {

  }

  void UIManager::cleanupForStateTransition()
  {
    components_.clear();
    active_binding_count_ = 0;
    invalidateComponentCache();

    value_cache_.clear();
    text_cache_.clear();

    layouts_.clear();

    global_style_ = UIStyle{};
    global_font_id_ = UIConstants::FONT_DEFAULT;
  }

  void UIManager::prepareForStateTransition()
  {
    cleanupForStateTransition();
  }

  void UIManager::clean()
  {
    if (is_shutdown_)
    {
      return;
    }

    cleanupForStateTransition();

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

  void UIManager::createLabel(const std::string& id, const UIRect& bounds, const std::string& text)
  {
    auto component = std::make_shared<UIComponent>();
    component->id = id;
    component->type = UIComponentType::LABEL;
    component->bounds = scaleRect(bounds);
    component->text = text;
    component->z_order = UIConstants::ZORDER_LABEL;

    components_[id] = component;

    calculateOptimalSize(component);
  }

  void UIManager::repositionAllComponents(int width, int height)
  {
    for (auto& [id, component] : components_)
    {
      if (component)
      {
        applyPositioning(component, width, height);
      }
    }
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
      bounds.x = (width - bounds.width) / 2 + scaled_offset_x;
      bounds.y = scaled_offset_y;
      break;
    case UIPositionMode::CENTERED_V:
      bounds.x = scaled_offset_x;
      bounds.y = (height - bounds.height) / 2 + scaled_offset_y;
      break;
    case UIPositionMode::CENTERED_BOTH:
      break;
    case UIPositionMode::TOP_ALIGNED:
      bounds.x = scaled_offset_x;
      bounds.y = scaled_offset_y;
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

  int UIManager::getLogicalWidth() const
  {
    const auto& game_engine = SimulacrumEngine::Instance();
    return game_engine.getLogicalWidth();
  }

  int UIManager::getLogicalHeight() const
  {
    const auto& game_engine = SimulacrumEngine::Instance();
    return game_engine.getLogicalHeight();
  }

  void UIManager::calculateOptimalSize(const std::string& id)
  {
    auto component = getComponent(id);
    if (component)
    {
      calculateOptimalSize(component);
    }
  }

  void UIManager::calculateOptimalSize(std::shared_ptr<UIComponent> component)
  {
    if (!component || !component->auto_size_)
    {
      return;
    }

    int content_width = 0;
    int content_height = 0;

    if (!measureComponentContent(component, &content_width, &content_height))
    {
      return;
    }

    int scaled_content_padding = static_cast<int>(component->content_padding * global_scale_);

    if (component->auto_width)
    {
      int total_width = content_width + (scaled_content_padding * 2);
      int old_width = component->bounds.width;
      component->bounds.width =
        std::max(component->min_bounds.width,
          std::min(total_width, component->max_bounds.width));

      if (component->style.text_align == UIAlignment::CENTER_CENTER &&
        component->bounds.width != old_width &&
        (component->type == UIComponentType::TITLE || component->type == UIComponentType::LABEL))
      {
        const auto& engine = SimulacrumEngine::Instance();
        int window_width = engine.getLogicalWidth();
        component->bounds.x = (window_width - component->bounds.width) / 2;
      }
    }

    if (component->on_content_changed)
    {
      component->on_content_changed();
    }
  }

  bool UIManager::measureComponentContent(const std::shared_ptr<UIComponent>& component, int* width, int* height)
  {
    if (!component || !width || !height)
    {
      return false;
    }

    auto& font_manager = FontManager::Instance();

    switch (component->type)
    {
      case UIComponentType::BUTTON:
      case UIComponentType::BUTTON_DANGER:
      case UIComponentType::BUTTON_SUCCESS:
      case UIComponentType::BUTTON_WARNING:
      case UIComponentType::LABEL:
      case UIComponentType::TITLE:
        if (!component->text.empty())
        {
          if (component->text.find('\n'))
          {
            return font_manager.measureMultilineText(
              component->text,
              component->style.font_id,
              0,
              width,
              height
            );
          }
          else
          {
            return font_manager.measureText(component->text, component->style.font_id, width, height);
          }
        }
        *width = component->min_bounds.width;
        *height = component->min_bounds.height;
        return true;

      default:
        // For other component types, use current bounds or minimums
        *width = std::max(component->bounds.width, component->min_bounds.width);
        *height = std::max(component->bounds.height, component->min_bounds.height);
        return true;
    }

    *width = component->min_bounds.width;
    *height = component->min_bounds.height;
    return true;
  }

  void UIManager::recordGPUVertices(GPURenderer& gpu_renderer)
  {
    gpu_primitive_commands.clear();
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

    auto& font_manager = FontManager::Instance();
    auto addText = [&](
      const std::string& text, const std::string& font_id,
      int x, int y, SDL_Color& color, int alignment,
      bool use_background = false, const SDL_Color& bg_color = { 0, 0, 0, 0 }, int bg_padding = 0
    )
      {
        if (text.empty())
        {
          spdlog::debug("Text empty");
          return;
        }

        const GPUTextData* text_data = font_manager.renderTextGPU(text, font_id, color);
        if (!text_data || !text_data->texture || !text_data->texture->isValid())
        {
          spdlog::debug("text data error");
          return;
        }

        if (ui_offset + 4 > GPU_UI_VERTEX_LIMIT)
        {
          spdlog::debug("UI offset exceeds GPU UI vertex limit");
          return;
        }

        float dst_x = static_cast<float>(x);
        float dst_y = static_cast<float>(y);
        float dst_w = static_cast<float>(text_data->width);
        float dst_h = static_cast<float>(text_data->height);

        switch (alignment)
        {
          case 1: // Left (vertically centered)
            dst_y -= dst_h / 2;
            break;
          case 2: // Right (vertically centered)
            dst_x -= dst_w;
            dst_y -= dst_h / 2;
            break;
          case 3: // Top-left
            // x and y stay as-is (top-left corner)
            break;
          case 4: // Top-center
            dst_x -= dst_w / 2;
            break;
          case 5: // Top-right
            dst_x -= dst_w;
            break;
          default: // Center (0)
            dst_x -= dst_w / 2;
            dst_y -= dst_h / 2;
            break;
        }

        // Draw background rectangle if enabled (add to primitive commands, renders before text)
        if (use_background && bg_color.a > 0)
        {
          int effective_padding = bg_padding;
          if (global_scale_ < 1.0f)
          {
            effective_padding = static_cast<int>(bg_padding * global_scale_);
          }

          UIRect bg_rect;
          bg_rect.x = static_cast<int>(dst_x) - effective_padding;
          bg_rect.y = static_cast<int>(dst_y) - effective_padding;
          bg_rect.width = static_cast<int>(dst_w) + (effective_padding * 2);
          bg_rect.height = static_cast<int>(dst_h) + (effective_padding * 2);
          addFilledRect(bg_rect, bg_color);
        }

        SpriteVertex* v = ui_base + ui_offset;
        v[0] = { dst_x, dst_y, 0.0f, 0.0f, color.r, color.g, color.b, color.a };
        v[1] = { dst_x + dst_w, dst_y, 1.0f, 0.0f, color.r, color.g, color.b, color.a };
        v[2] = { dst_x + dst_w, dst_y + dst_h, 1.0f, 1.0f, color.r, color.g, color.b, color.a };
        v[3] = { dst_x, dst_y + dst_h, 0.0f, 1.0f, color.r, color.g, color.b, color.a };

        UIGPUDrawCommand cmd;
        cmd.type = UIGPUDrawCommand::Type::Text;
        cmd.texture = text_data->texture->get();
        cmd.vertex_count = 4;
        gpu_text_commands.push_back(cmd);
        ui_offset += 4;
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

      case UIComponentType::LABEL:
      case UIComponentType::TITLE:
        if (!component->text.empty())
        {
          int text_x;
          int text_y;
          int alignment;

          int scaled_padding = static_cast<int>(component->style.padding * global_scale_);

          switch (component->style.text_align)
          {
            case UIAlignment::CENTER_CENTER:
              text_x = component->bounds.x + component->bounds.width / 2;
              text_y = component->bounds.y + component->bounds.height / 2;
              alignment = 0;
              break;
            case UIAlignment::CENTER_RIGHT:
              text_x = component->bounds.x + component->bounds.width - scaled_padding;
              text_y = component->bounds.y + component->bounds.height / 2;
              alignment = 2;
              break;
            case UIAlignment::CENTER_LEFT:
              text_x = component->bounds.x + scaled_padding;
              text_y = component->bounds.y + component->bounds.height / 2;
              alignment = 1;
              break;
            case UIAlignment::TOP_CENTER:
              text_x = component->bounds.x + component->bounds.width / 2;
              text_y = component->bounds.y + scaled_padding;
              alignment = 4;
              break;
            case UIAlignment::TOP_LEFT:
              text_x = component->bounds.x + scaled_padding;
              text_y = component->bounds.y + scaled_padding;
              alignment = 3;
              break;
            case UIAlignment::TOP_RIGHT:
              text_x = component->bounds.x + component->bounds.width - scaled_padding;
              text_y = component->bounds.y + scaled_padding;
              alignment = 5;
              break;
            default:
              // CENTER_LEFT is default
              text_x = component->bounds.x + scaled_padding;
              text_y = component->bounds.y + component->bounds.height / 2;
              alignment = 1;
              break;
          }

          bool needs_background =
            component->style.use_text_background &&
            component->style.background_color.a == 0;

          int scaled_text_bg_padding = static_cast<int>(component->style.text_background_padding * global_scale_);
          addText(
            component->text, component->style.font_id,
            text_x, text_y, component->style.text_color, alignment,
            needs_background, component->style.text_background_color,
            scaled_text_bg_padding
          );
        }
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

    if (!gpu_text_commands.empty())
    {
      SDL_BindGPUGraphicsPipeline(swapchain_pass, gpu_renderer.getUISpritePipeline());
      gpu_renderer.pushViewProjection(swapchain_pass, ortho_matrix);

      SDL_GPUBufferBinding vertex_binding{};
      vertex_binding.buffer = gpu_renderer.getUIVertexPool().getGPUBuffer();
      vertex_binding.offset = 0;
      SDL_BindGPUVertexBuffers(swapchain_pass, 0, &vertex_binding, 1);

      auto& batch = gpu_renderer.getSpriteBatch();
      SDL_GPUBufferBinding index_binding{};
      index_binding.buffer = batch.getIndexBuffer();
      index_binding.offset = 0;
      SDL_BindGPUIndexBuffer(swapchain_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

      for (const auto& cmd : gpu_text_commands)
      {
        SDL_GPUTextureSamplerBinding tex_sampler{};
        tex_sampler.texture = cmd.texture;
        tex_sampler.sampler = gpu_renderer.getLinearSampler();
        SDL_BindGPUFragmentSamplers(swapchain_pass, 0, &tex_sampler, 1);

        uint32_t first_index = (cmd.vertex_offset / 4) * 6;
        SDL_DrawGPUIndexedPrimitives(swapchain_pass, 6, 1, first_index, 0, 0);
      }
    }
  }

  std::shared_ptr<UIComponent> UIManager::getComponent(const std::string& id)
  {
    auto it = components_.find(id);
    return (it != components_.end()) ? it->second : nullptr;
  }

  std::shared_ptr<const UIComponent> UIManager::getComponent(const std::string& id) const
  {
    auto it = components_.find(id);
    return (it != components_.end()) ? it->second : nullptr;
  }

  void UIManager::onWindowResize(int logical_width, int logical_height)
  {
    global_scale_ = calculateOptimalScale(logical_width, logical_height);
    repositionAllComponents(logical_width, logical_height);
    current_logical_width_ = logical_width;
    current_logical_height_ = logical_height;
  }

  void UIManager::setGlobalScale(float scale)
  {
    global_scale_ = scale;
  }

  float UIManager::calculateOptimalScale(int width, int height) const
  {
    float scale = std::min(width / UIConstants::BASELINE_WIDTH_F, height / UIConstants::BASELINE_HEIGHT_F);
    return std::min(UIConstants::MAX_UI_SCALE, scale);
  }

  void UIManager::invalidateComponentCache()
  {
    spdlog::debug("Invalidating component cache");
    sorted_components_dirty_ = true;
  }

  void UIManager::loadTheme(const UITheme& theme)
  {
    current_theme_ = theme;

    for (const auto& [id, component] : components_)
    {
      if (component)
      {
        UIAlignment preserved_alignment = component->style.text_align;
        component->style = current_theme_.getStyle(component->type);
        component->style.text_align = preserved_alignment;
      }
    }
  }

  void UIManager::setDefaultTheme()
  {
    setDarkTheme();
  }

  void UIManager::setDarkTheme()
  {
    UITheme dark_theme;
    dark_theme.name = "dark";
    current_theme_mode = "dark";

    // Button Style

    // Button Danger Style

    // Button Success Style

    // Button Warning Style

    // Label Style
    UIStyle label_style;
    label_style.background_color = {0, 0, 0, 0};
    label_style.text_color = {255, 255, 255, 255};
    label_style.text_align = UIAlignment::CENTER_LEFT;
    label_style.font_id = UIConstants::FONT_UI;

    label_style.use_text_background = true;
    label_style.text_background_color = {0, 0, 0, 100};
    label_style.text_background_padding = UIConstants::LABEL_TEXT_BG_PADDING;

    dark_theme.component_styles[UIComponentType::LABEL] = label_style;

    // Panel Style
    UIStyle panel_style;
    panel_style.background_color = {0, 0, 0, 50};
    panel_style.border_width = UIConstants::BORDER_WIDTH_NONE;
    panel_style.font_id = UIConstants::FONT_UI;

    dark_theme.component_styles[UIComponentType::PANEL] = panel_style;
  }

} // namespace Simulacrum
