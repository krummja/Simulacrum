#include "ResourcePath.hpp"
#include "GpuRenderer.hpp"
#include "GpuShaderManager.hpp"
#include <cstring>
#include <format>
#include <spdlog/spdlog.h>

namespace Simulacrum
{

  GPURenderer& GPURenderer::Instance()
  {
    static GPURenderer instance;
    return instance;
  }

  bool GPURenderer::init()
  {
    if (initialized_)
    {
      spdlog::warn("GPURenderer already initialized");
      return true;
    }

    auto& gpu_device = GPUDevice::Instance();
    if (!gpu_device.isInitialized())
    {
      spdlog::error("GPURenderer::init: GPUDevice not initialized");
      return false;
    }

    device_ = gpu_device.get();
    window_ = gpu_device.getWindow();

    // Get window size for viewport (logical size - matches swapchain)
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(window_, &w, &h);
    viewport_width_ = static_cast<uint32_t>(w);
    viewport_height_ = static_cast<uint32_t>(h);

    // Initialize shader manager
    if (!GPUShaderManager::Instance().init(device_))
    {
      spdlog::error("GPURenderer: failed to init shader manager");
      return false;
    }

    // Create samplers
    nearest_sampler_ = GPUSampler::createNearest(device_);
    linear_sampler_ = GPUSampler::createLinear(device_);

    if (!nearest_sampler_.isValid() || !linear_sampler_.isValid())
    {
      spdlog::error("GPURenderer: failed to create samplers");
      cleanupPartialInit();
      return false;
    }

    // Create scene texture
    if (!createSceneTexture())
    {
      spdlog::error("GPURenderer: failed to create scene texture");
      cleanupPartialInit();
      return false;
    }

    // Load shader and create pipelines
    if (!loadShaders())
    {
      spdlog::error("GPURenderer: failed to create shaders");
      cleanupPartialInit();
      return false;
    }

    if (!createPipelines())
    {
      spdlog::error("GPURenderer: failed to create pipelines");
      cleanupPartialInit();
      return false;
    }

    // Initialize vertex pools
    if (!primitive_vertex_pool_.init(device_, sizeof(ColorVertex), 300000))
    {
      spdlog::error("GPURenderer: failed to init primitive vertex pool");
      cleanupPartialInit();
      return false;
    }

    if (!ui_vertex_pool_.init(device_, sizeof(SpriteVertex), 4000))
    {
      spdlog::error("GPURenderer: failed to init UI vertex pool");
      cleanupPartialInit();
      return false;
    }

    if (!sprite_batch_.init(device_))
    {
      spdlog::error("GPURenderer: failed to init sprite batch");
      cleanupPartialInit();
      return false;
    }

    initialized_ = true;
    spdlog::info("GPURenderer initialized: {}x{}", viewport_width_, viewport_height_);
    return true;
  }

  void GPURenderer::shutdown()
  {
    if (!initialized_)
    {
      return;
    }

    // Release sprite batches
    sprite_batch_.shutdown();

    // Release vertex pools
    primitive_vertex_pool_.shutdown();
    ui_vertex_pool_.shutdown();

    // Release pipelines
    primitive_pipeline_.release();
    ui_sprite_pipeline_.release();
    ui_primitive_pipeline_.release();

    // Release textures and samplers
    scene_texture_.reset();
    nearest_sampler_ = GPUSampler();
    linear_sampler_ = GPUSampler();

    // Shutdown shader manager
    GPUShaderManager::Instance().shutdown();

    device_ = nullptr;
    window_ = nullptr;
    initialized_ = false;

    spdlog::info("GPURenderer shutdown complete");
  }

  void GPURenderer::cleanupPartialInit()
  {
    sprite_batch_.shutdown();

    // Release vertex pools
    primitive_vertex_pool_.shutdown();
    ui_vertex_pool_.shutdown();

    // Release pipelines
    primitive_pipeline_.release();
    ui_sprite_pipeline_.release();
    ui_primitive_pipeline_.release();

    // Release textures and samplers
    scene_texture_.reset();
    nearest_sampler_ = GPUSampler();
    linear_sampler_ = GPUSampler();

    // Shutdown shader manager
    GPUShaderManager::Instance().shutdown();

    device_ = nullptr;
    window_ = nullptr;
  }

  void GPURenderer::beginFrame()
  {
    if (!initialized_)
    {
      return;
    }

    command_buffer_ = SDL_AcquireGPUCommandBuffer(device_);

    if (!command_buffer_)
    {
      spdlog::error("Failed to acquire GPU command buffer: {}", SDL_GetError());
      return;
    }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
      command_buffer_,
      window_,
      &swapchain_texture_,
      &swapchain_width_,
      &swapchain_height_
    ))
    {
      spdlog::error("Failed to acquire swapchain texture: {}", SDL_GetError());
      swapchain_texture_ = nullptr;
      SDL_CancelGPUCommandBuffer(command_buffer_);
      command_buffer_ = nullptr;
      return;
    }

    if (!swapchain_texture_)
    {
      SDL_CancelGPUCommandBuffer(command_buffer_);
      command_buffer_ = nullptr;
      return;
    }

    // Sync viewport to swapchain size (authoritative source from resize events)
    if (swapchain_width_ != viewport_width_ || swapchain_height_ != viewport_height_)
    {
      spdlog::info("Swapchain size changed: {}x{} -> {}x{}",
        viewport_width_, viewport_height_,
        swapchain_width_, swapchain_height_
      );

      updateViewport(swapchain_width_, swapchain_height_);
    }

    // Begin vertex pool frames (maps transfer buffers)
    primitive_vertex_pool_.beginFrame();
    ui_vertex_pool_.beginFrame();

    copy_pass_ = SDL_BeginGPUCopyPass(command_buffer_);
  }

  SDL_GPURenderPass* GPURenderer::beginScenePass()
  {
    if (!command_buffer_)
    {
      return nullptr;
    }

    // End copy pass
    if (copy_pass_)
    {
      // Process pending texture uploads.
      // TODO

      // End vertex pool frames (unmaps buffers for upload)
      size_t sprite_vertex_count = sprite_batch_.getVertexCount();
      if (sprite_vertex_count == 0)
      {
        sprite_vertex_count = sprite_vertex_pool_.getPendingVertexCount();
      }
      sprite_vertex_pool_.endFrame(sprite_vertex_count);

      size_t primitive_vertex_count = primitive_vertex_pool_.getPendingVertexCount();
      primitive_vertex_pool_.endFrame(primitive_vertex_count);

      size_t ui_vertex_count = ui_vertex_pool_.getPendingVertexCount();
      ui_vertex_pool_.endFrame(ui_vertex_count);

      // Upload vertex data
      sprite_vertex_pool_.upload(copy_pass_);
      primitive_vertex_pool_.upload(copy_pass_);
      ui_vertex_pool_.upload(copy_pass_);

      SDL_EndGPUCopyPass(copy_pass_);
      copy_pass_ = nullptr;
    }

    // Begin scene render pass
    SDL_GPUColorTargetInfo color_target = scene_texture_->asColorTarget(
      SDL_GPU_LOADOP_CLEAR,
      { 0.95f, 0.02f, 0.30f, 1.0f }
    );

    current_pass_ = SDL_BeginGPURenderPass(command_buffer_, &color_target, 1, nullptr);

    // Set viewport to match scene texture dimensions
    // Scene texture is 3x viewport for zoom headroom
    uint32_t scene_w = scene_texture_->getWidth();
    uint32_t scene_h = scene_texture_->getHeight();

    SDL_GPUViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.w = static_cast<float>(scene_w);
    viewport.h = static_cast<float>(scene_h);
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;
    SDL_SetGPUViewport(current_pass_, &viewport);

    return current_pass_;
  }

  SDL_GPURenderPass* GPURenderer::beginSwapchainPass()
  {
    if (!command_buffer_)
    {
      return nullptr;
    }

    // End scene pass
    if (current_pass_)
    {
      SDL_EndGPURenderPass(current_pass_);
      current_pass_ = nullptr;
    }

    if (!swapchain_texture_)
    {
      return nullptr;
    }

    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = swapchain_texture_;
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;
    color_target.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

    current_pass_ = SDL_BeginGPURenderPass(command_buffer_, &color_target, 1, nullptr);

    SDL_GPUViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.w = static_cast<float>(swapchain_width_);
    viewport.h = static_cast<float>(swapchain_height_);
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;
    SDL_SetGPUViewport(current_pass_, &viewport);

    return current_pass_;
  }

  void GPURenderer::endFrame()
  {
    if (!command_buffer_)
    {
      return;
    }

    // End active render pass
    if (current_pass_)
    {
      SDL_EndGPURenderPass(current_pass_);
      current_pass_ = nullptr;
    }

    // End copy pass if still active
    if (copy_pass_)
    {
      SDL_EndGPUCopyPass(copy_pass_);
      copy_pass_ = nullptr;
    }

    // Submit command buffer (measure to detect backpressure)
    SDL_SubmitGPUCommandBuffer(command_buffer_);

    command_buffer_ = nullptr;
    swapchain_texture_ = nullptr;
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getPrimitivePipeline() const
  {
    return primitive_pipeline_.get();
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getUISpritePipeline() const
  {
    return ui_sprite_pipeline_.get();
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getUIPrimitivePipeline() const
  {
    return ui_primitive_pipeline_.get();
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getCompositePipeline() const
  {
    return composite_pipeline_.get();
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getSpriteOpaquePipeline() const
  {
    return sprite_opaque_pipeline_.get();
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getSpriteAlphaPipeline() const
  {
    return sprite_alpha_pipeline_.get();
  }

  SDL_GPUGraphicsPipeline* GPURenderer::getParticlePipeline() const
  {
    return particle_pipeline_.get();
  }

  void GPURenderer::updateViewport(uint32_t width, uint32_t height)
  {
    if (width == 0 || height == 0)
    {
      spdlog::warn("GPURenderer::updateViewport() ignored invalid size {}x{}", width, height);
      return;
    }

    if (width == viewport_width_ && height == viewport_height_)
    {
      return;
    }

    uint32_t old_width = viewport_width_;
    uint32_t old_height = viewport_height_;

    viewport_width_ = width;
    viewport_height_ = height;

    if (!createSceneTexture())
    {
      spdlog::error("GPURenderer::updateViewport() failed to recreate scene texture for {}x{}", width, height);
      viewport_width_ = old_width;
      viewport_height_ = old_height;
      return;
    }
  }

  void GPURenderer::pushViewProjection(SDL_GPURenderPass* pass, const float* view_projection)
  {
    if (!pass || !view_projection)
    {
      return;
    }

    SDL_PushGPUVertexUniformData(command_buffer_, 0, view_projection, sizeof(float) & 16);
  }

  void GPURenderer::pushCompositeUniforms(SDL_GPURenderPass* pass, float subpixelX, float subpixelY, float zoom)
  {
    if (!pass)
    {
      return;
    }

    CompositeUBO ubo{};
    ubo.subPixelOffsetX = subpixelX;
    ubo.subPixelOffsetY = subpixelY;
    ubo.zoom = zoom;
    ubo._pad0 = 0.0f;

    SDL_PushGPUFragmentUniformData(command_buffer_, 0, &ubo, sizeof(CompositeUBO));
  }

  void GPURenderer::setCompositeParams(float zoom, float subpixelX, float subpixelY)
  {
    composite_zoom_ = zoom;
    composite_subpixel_x_ = subpixelX;
    composite_subpixel_y_ = subpixelY;
  }

  void GPURenderer::renderComposite(SDL_GPURenderPass* pass)
  {
    if (!pass || !scene_texture_ || !scene_texture_->isValid())
    {
      return;
    }

    // Bind the composite pipeline
    SDL_BindGPUGraphicsPipeline(pass, composite_pipeline_.get());

    // Bind scene texture with linear sampler for smooth compositing
    SDL_GPUTextureSamplerBinding tex_sampler{};
    tex_sampler.texture = scene_texture_->get();
    tex_sampler.sampler = linear_sampler_.get();
    SDL_BindGPUFragmentSamplers(pass, 0, &tex_sampler, 1);

    // Push composite uniforms (using stored params)
    pushCompositeUniforms(pass, composite_subpixel_x_, composite_subpixel_y_, composite_zoom_);

    // Draw fullscreen triangle (3 vertices, no vertex buffer needed)
    // The composite vertex shader uses gl_VertexIndex to generate positions
    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
  }

  void GPURenderer::createOrthoMatrix(float left, float right, float bottom, float top, float* out)
  {
    std::memset(out, 0, sizeof(float) * 16);

    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -1.0f;
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[15] = 1.0f;
  }

  bool GPURenderer::loadShaders()
  {
    auto& shader_manager = GPUShaderManager::Instance();

    ShaderInfo sprite_vert_info{};
    sprite_vert_info.num_uniform_buffers = 1;

    ShaderInfo sprite_frag_info{};
    sprite_frag_info.num_samplers = 1;

    ShaderInfo color_vert_info{};
    color_vert_info.num_uniform_buffers = 1;

    ShaderInfo color_frag_info{};

    ShaderInfo composite_vert_info{};

    ShaderInfo composite_frag_info{};
    composite_frag_info.num_samplers = 1;
    composite_frag_info.num_uniform_buffers = 1;  // CompositeUBO

    if (!shader_manager.loadShader(ResourcePath::resolve("res/shaders/sprite.vert"), SDL_GPU_SHADERSTAGE_VERTEX, sprite_vert_info))
    {
      return false;
    }

    if (!shader_manager.loadShader(ResourcePath::resolve("res/shaders/sprite.frag"), SDL_GPU_SHADERSTAGE_FRAGMENT, sprite_frag_info))
    {
      return false;
    }

    if (!shader_manager.loadShader(ResourcePath::resolve("res/shaders/color.vert"), SDL_GPU_SHADERSTAGE_VERTEX, color_vert_info))
    {
      return false;
    }

    if (!shader_manager.loadShader(ResourcePath::resolve("res/shaders/color.frag"), SDL_GPU_SHADERSTAGE_FRAGMENT, color_frag_info))
    {
      return false;
    }

    if (!shader_manager.loadShader(ResourcePath::resolve("res/shaders/composite.vert"), SDL_GPU_SHADERSTAGE_VERTEX, composite_vert_info))
    {
      return false;
    }

    if (!shader_manager.loadShader(ResourcePath::resolve("res/shaders/composite.frag"), SDL_GPU_SHADERSTAGE_FRAGMENT, composite_frag_info))
    {
      return false;
    }

    return true;
  }

  bool GPURenderer::createPipelines()
  {
    auto& shader_manager = GPUShaderManager::Instance();

    SDL_GPUTextureFormat scene_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    SDL_GPUTextureFormat swapchain_format = GPUDevice::Instance().getSwapchainFormat();

    const std::string sprite_vert = ResourcePath::resolve("res/shaders/sprite.vert");
    const std::string sprite_frag = ResourcePath::resolve("res/shaders/sprite.frag");
    const std::string color_vert = ResourcePath::resolve("res/shaders/color.vert");
    const std::string color_frag = ResourcePath::resolve("res/shaders/color.frag");
    const std::string composite_vert = ResourcePath::resolve("res/shaders/composite.vert");
    const std::string composite_frag = ResourcePath::resolve("res/shaders/composite.frag");

    {
      auto config = GPUPipeline::createPrimitiveConfig(
        shader_manager.getShader(color_vert),
        shader_manager.getShader(color_frag),
        scene_format
      );

      if (!primitive_pipeline_.create(device_, config))
      {
        spdlog::error("Failed to create {} pipeline", "primitive");
        return false;
      }
    }

    {
      auto config = GPUPipeline::createCompositeConfig(
        shader_manager.getShader(composite_vert),
        shader_manager.getShader(composite_frag),
        swapchain_format
      );

      if (!composite_pipeline_.create(device_, config))
      {
        spdlog::error("Failed to create {} pipeline", "composite");
        return false;
      }
    }

    {
      auto config = GPUPipeline::createSpriteConfig(
        shader_manager.getShader(sprite_vert),
        shader_manager.getShader(sprite_frag),
        swapchain_format,
        true  // alpha blending for text
      );

      if (!ui_sprite_pipeline_.create(device_, config))
      {
        spdlog::error("Failed to create {} pipeline", "ui_sprite");
        return false;
      }
    }

    {
      auto config = GPUPipeline::createPrimitiveConfig(
        shader_manager.getShader(color_vert),
        shader_manager.getShader(color_frag),
        swapchain_format
      );

      if (!ui_primitive_pipeline_.create(device_, config))
      {
        spdlog::error("Failed to create {} pipeline", "ui_primitive");
        return false;
      }
    }

    return true;
  }

  bool GPURenderer::createSceneTexture()
  {
    uint32_t scene_width = viewport_width_;
    uint32_t scene_height = viewport_height_;

    scene_texture_ = std::make_unique<GPUTexture>(
      device_,
      scene_width,
      scene_height,
      SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
      SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER
    );

    if (!scene_texture_->isValid())
    {
      spdlog::error("Failed to create scene texture {}x{}", scene_width, scene_height);
      return false;
    }

    return true;
  }

} // namespace Simulacrum
