#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"

#include "SimulacrumEngine.hpp"
#include "SettingsManager.hpp"
#include "InputManager.hpp"
#include "GpuDevice.hpp"
#include "GpuRenderer.hpp"
#include "ResourcePath.hpp"
#include "ThreadSystem.hpp"
#include "StateManager.hpp"
#include "UIManager.hpp"
#include "TextureManager.hpp"
#include "GpuTexture.hpp"

#include "StateLoading.hpp"

#include <cstdlib>
#include <format>
#include <future>
#include <string>
#include <string_view>
#include <vector>
#include <spdlog/spdlog.h>

namespace Simulacrum
{

#define SIMULACRUM_DARK 31, 31, 31, 255
#define SIMULACRUM_SAKURA 240, 5, 74, 255

  bool SimulacrumEngine::init(std::string_view title)
  {
    spdlog::info("Initializing SDL video and gamepad");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
      spdlog::error("SDL initialization failed: {}", SDL_GetError());
      return false;
    }

    spdlog::info("SDL video online");

    // Initialize resource path resolver
    ResourcePath::init();

    constexpr int DEFAULT_WIDTH = 1280;
    constexpr int DEFAULT_HEIGHT = 720;

    spdlog::info("Loading settings");
    const std::string settings_path = ResourcePath::resolve("res/settings.json");
    auto& settings_manager = SettingsManager::Instance();
    settings_manager.loadFromFile(settings_path);

    auto& graphics_settings = settings_manager.getGraphicsSettings();

    const int width = graphics_settings.resolution_width;
    const int height = graphics_settings.resolution_height;
    bool fullscreen = graphics_settings.fullscreen;

    SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "3");
    SDL_SetHint("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", "0");
    SDL_SetHint("SDL_MOUSE_AUTO_CAPTURE", "0");
    SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "0");
    SDL_SetHint("SDL_RENDER_BATCHING", "1");

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint(SDL_HINT_VIDEO_DOUBLE_BUFFER, "1");
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");

    if (width <= 0 || height <= 0)
    {
      window_width_ = 1280;
      window_height_ = 720;
    }
    else
    {
      window_width_ = width;
      window_height_ = height;
    }

    windowed_width_ = window_width_;
    windowed_height_ = window_height_;

    SDL_WindowFlags flags = 0;

    if (fullscreen)
    {
      flags |= SDL_WINDOW_FULLSCREEN;
    }

    is_fullscreen_ = fullscreen;

    window_.reset(
      SDL_CreateWindow(
        title.data(),
        window_width_,
        window_height_,
        flags
      )
    );

    if (!window_)
    {
      spdlog::error("Failed to create window: {}", SDL_GetError());
      return false;
    }

    spdlog::info("Window creation system online");

    // Initialize GPU device and renderer
    auto& gpu_device = GPUDevice::Instance();

    if (gpu_device.init(window_.get()))
    {
      auto& gpu_renderer = GPURenderer::Instance();
      if (gpu_renderer.init())
      {
        spdlog::info("SDL3_GPU rendering initialized successfully");
      }
      else
      {
        spdlog::warn("GPURenderer init failed - falling back to SDL_Renderer");
        gpu_device.shutdown();
      }
    }
    else
    {
      spdlog::warn("GPUDevice init failed - falling back to SDL_Renderer");
    }

    // Set window icon
    // TODO

    // Cache window sizes once for subsequent initialization steps
    int pixel_width = window_width_;
    int pixel_height = window_height_;
    int logical_width = window_width_;
    int logical_height = window_height_;

    if (!SDL_GetWindowSizeInPixels(window_.get(), &pixel_width, &pixel_height))
    {
      spdlog::error("Failed to get window pixel size: {}", SDL_GetError());
    }

    if (!SDL_GetWindowSizeInPixels(window_.get(), &logical_width, &logical_height))
    {
      spdlog::error("Failed to get window logical size: {}", SDL_GetError());
    }

    spdlog::info("GPU rendering system online");

    // Unified VSync initialization with automatic fallback
    auto& settings = SettingsManager::Instance();
    bool vsync_requested = settings.getGraphicsSettings().vsync;
    vsync_requested_ = vsync_requested;

    spdlog::info(
      "VSync setting from SettingsManager: {}",
      vsync_requested ? "enabled" : "disabled"
    );

    // Create TimestepManager (uses default 60 FPS target and 1/60s fixed timestep)
    timestep_manager_ = std::make_unique<TimestepManager>();

    if (timestep_manager_->isUsingSoftwareFrameLimiting())
    {
      spdlog::info(
        "Created: {:.0f} Hz updates, {:.0f} FPS target, software frame limiting",
        timestep_manager_->getUpdateFrequencyHz(),
        timestep_manager_->getTargetFPS()
      );
    }
    else
    {
      spdlog::info(
        "Created: {:.0f} Hz updates, VSync enabled",
        timestep_manager_->getUpdateFrequencyHz()
      );
    }

    // Store actual dimensions for UI positioning
    int const actual_width = pixel_width;
    int const actual_height = pixel_height;
    logical_width = actual_width;
    logical_height = actual_height;

    spdlog::info("Using native resolution: {}x{}", actual_width, actual_height);

    // Check if icon loaded successfully
    // TODO

    // INITIALIZING GAME RESOURCE LOADING AND MANAGEMENT

    // Calculate DPI-aware font sizes before threading
    dpi_scale_ = 1.0f;

    spdlog::info(
      "DPI scale: {}, window: {}x{}",
      dpi_scale_, window_width_, window_height_
    );

    // Use multiple threads for initialization
    std::vector<std::future<bool>> init_tasks;  // initialization tasks vector
    init_tasks.reserve(12);  // reserve capacity for typical number of init tasks

    init_tasks.push_back(
      ThreadSystem::Instance().enqueueTaskWithResult(
        []() -> bool
        {
          InputManager& input_manager = InputManager::Instance();
          if (!input_manager.init())
          {
            spdlog::critical("Failed to initialize Input Manager");
            return false;
          }

          return true;
        }
      )
    );

    // Create and initialize texture manager - MAIN THREAD
    spdlog::info("Create texture manager");

    TextureManager& texture_manager = TextureManager::Instance();

    // Load textures in main thread
    spdlog::info("Creating and loading textures");
    const std::string texture_res_path = ResourcePath::resolve("res/img");
    constexpr std::string_view texture_prefix = "";

    texture_manager.loadGPU(texture_res_path, std::string(texture_prefix));

    // TODO Sound Manager

    // TODO Font Manager

    // TODO Save Game Manager

    // TODO Collision Manager

    // TODO AI Manager

    // TODO Particle Manager

    // TODO Resource Template Manager

    // TODO World Manager

    state_manager_ = std::make_unique<StateManager>();

    // UIManager
    UIManager& ui_mgr = UIManager::Instance();
    if (!ui_mgr.init())
    {
      spdlog::critical("Failed to initialize UI Manager");
      return false;
    }

    spdlog::debug("UI Manager initialized successfully");

    // Load states

    state_manager_->addState(std::make_unique<LoadingState>());

    bool all_tasks_succeeded = true;

    // Retrieve each init task and check status
    for (auto& task : init_tasks)
    {
      try
      {
        all_tasks_succeeded &= task.get();
      }
      catch (const std::exception& exc)
      {
        all_tasks_succeeded = false;
      }
    }

    if (!all_tasks_succeeded)
    {
      return false;
    }

    running_ = true;
    return true;
  }

  void SimulacrumEngine::handleEvents()
  {
    InputManager& input_manager = InputManager::Instance();

    input_manager.clearFrameInput();

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
      // SDL_ConvertEventToRenderCoordinates(renderer_.get(), &event);

      switch (event.type)
      {
      case SDL_EVENT_QUIT:
        setRunning(false);
        break;
      case SDL_EVENT_KEY_DOWN:
        input_manager.onKeyDown(event);
        break;
      case SDL_EVENT_KEY_UP:
        input_manager.onKeyUp(event);
        break;
      case SDL_EVENT_MOUSE_MOTION:
        input_manager.onMouseMove(event);
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        input_manager.onMouseButtonDown(event);
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        input_manager.onMouseButtonUp(event);
        break;
      case SDL_EVENT_WINDOW_RESIZED:
        onWindowResize(event);
        break;
      case SDL_EVENT_WINDOW_MINIMIZED:
      case SDL_EVENT_WINDOW_OCCLUDED:
      case SDL_EVENT_WINDOW_HIDDEN:
      case SDL_EVENT_WINDOW_FOCUS_LOST:
      case SDL_EVENT_WINDOW_RESTORED:
      case SDL_EVENT_WINDOW_SHOWN:
      case SDL_EVENT_WINDOW_EXPOSED:
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
        onWindowEvent(event);
        break;
      case SDL_EVENT_DISPLAY_ORIENTATION:
      case SDL_EVENT_DISPLAY_ADDED:
      case SDL_EVENT_DISPLAY_REMOVED:
      case SDL_EVENT_DISPLAY_MOVED:
      case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
        onDisplayChange(event);
        break;

      default:
        break;
      }
    }

    if (input_manager.isKeyJustPressed(SDL_SCANCODE_F1))
    {
      toggleFullscreen();
    }

    if (input_manager.isKeyJustPressed(SDL_SCANCODE_F3))
    {
      spdlog::info("Overlay Toggle");
    }

    state_manager_->handleInput();
  }

  void SimulacrumEngine::setRunning(bool running) { running_ = running; }

  float SimulacrumEngine::getCurrentFPS() const
  {
    return 0.0f;
  }

  void SimulacrumEngine::update(float delta_time)
  {
    state_manager_->setCurrentFPS(timestep_manager_->getCurrentFPS());
    state_manager_->update(delta_time);
  }

  void SimulacrumEngine::render()
  {
    float interpolation_alpha = static_cast<float>(timestep_manager_->getInterpolationAlpha());

    auto& gpu_renderer = GPURenderer::Instance();
    gpu_renderer.beginFrame();

    // Record vertices
    draw_commands_.clear();

    TextureManager& texture_manager = TextureManager::Instance();

    auto& vertex_pool = gpu_renderer.getSpriteVertexPool();
    auto* write_ptr = static_cast<SpriteVertex*>(vertex_pool.getMappedPtr());

    if (!write_ptr) return;

    uint32_t vertex_offset = 0;

    auto addImage = [&](const char* texture_name, int x, int y) {
      const TextureData* tex_data = texture_manager.getGPUTextureData(texture_name);
      if (!tex_data || !tex_data->texture) return;

      SpriteVertex* v = write_ptr + vertex_offset;

      float sx = static_cast<float>(x);
      float sy = static_cast<float>(y);

      float sw = tex_data->width;
      float sh = tex_data->height;

      v[0] = { sx, sy, 0.0f, 1.0f, 255, 255, 255, 255 };
      v[1] = { sx + sw, sy, 1.0f, 0.0f, 255, 255, 255, 255  };
      v[2] = { sx + sw, sy + sh, 1.0f, 1.0f, 255, 255, 255, 255 };
      v[3] = { sx, sy + sh, 0.0f, 1.0f, 255, 255, 255, 255 };

      GPUDrawCommand cmd;
      cmd.texture = tex_data->texture->get();
      cmd.vertex_count = 4;
      cmd.vertex_offset = 4;
      draw_commands_.push_back(cmd);
      vertex_offset += 4;
    };

    addImage("tile_0815", 0, 0);
    vertex_pool.setWrittenVertexCount(vertex_offset);

    SDL_GPURenderPass* scene_pass = gpu_renderer.beginScenePass();
    SDL_GPURenderPass* swapchain_pass = gpu_renderer.beginSwapchainPass();

    // Issue draw commands
  }

  void SimulacrumEngine::present()
  {
    GPURenderer::Instance().endFrame();
  }

  void SimulacrumEngine::processBackgroundTasks()
  {
    // TODO
  }

  void SimulacrumEngine::setLogicalPresentationMode(SDL_RendererLogicalPresentation mode)
  {
    // TODO
  }

  bool SimulacrumEngine::isVSyncEnabled() const noexcept
  {
    return false;
  }

  SDL_RendererLogicalPresentation SimulacrumEngine::getLogicalPresentationMode() const noexcept
  {
    return logical_presentation_mode_;
  }

  void SimulacrumEngine::clean()
  {
    // TODO Double check cleanup after init has been finished

    spdlog::info("Starting shutdown sequence...");

    auto window_to_destroy = std::move(window_);
    // auto renderer_to_destroy = std::move(renderer_);

    spdlog::info("Shutting down GPU renderer...");
    GPURenderer::Instance().shutdown();

    spdlog::info("Shutting down GPU device...");
    GPUDevice::Instance().shutdown();

    spdlog::info("Destroying window...");
    window_to_destroy.reset();
    spdlog::info("Window destroyed successfully");

    spdlog::info("Calling SDL_Quit...");
    SDL_Quit();

    spdlog::info("Shutdown complete!");
  }

  bool SimulacrumEngine::setVSyncEnabled(bool enable)
  {
    return false;
  }

  void SimulacrumEngine::toggleFullscreen()
  {
    if (!window_)
    {
      spdlog::error("Cannot toggle fullscreen - window not initialized");
      return;
    }

    is_fullscreen_ = !is_fullscreen_;

    if (!SDL_SetWindowFullscreen(window_.get(), is_fullscreen_))
    {
      spdlog::error("Failed to toggle fullscreen: {}", SDL_GetError());
      // Revert state on failure
      is_fullscreen_ = !is_fullscreen_;
      return;
    }

    // Restore window size when exiting fullscreen
    if (!is_fullscreen_)
    {
      if (!SDL_SetWindowSize(window_.get(), windowed_width_, windowed_height_))
      {
        spdlog::error("Failed to restore window size: {}", SDL_GetError());
      }
    }
  }

  void SimulacrumEngine::setFullscreen(bool enabled)
  {
    if (!window_)
    {
      spdlog::error("Cannot set fullscreen - window not initialized");
      return;
    }

    if (is_fullscreen_ == enabled)
    {
      return;
    }

    toggleFullscreen();
  }

  void SimulacrumEngine::setGlobalPause(bool paused)
  {
    // TODO
  }

  bool SimulacrumEngine::isGlobalPaused() const { return globally_paused_; }

  bool SimulacrumEngine::verifyVsyncState(bool requested)
  {
    return false;
  }

  void SimulacrumEngine::onWindowResize(const SDL_Event& event)
  {
    int const new_width = event.window.data1;
    int const new_height = event.window.data2;

    setWindowSize(new_width, new_height);

    int actual_width;
    int actual_height;
    if (!SDL_GetWindowSizeInPixels(window_.get(), &actual_width, &actual_height))
    {
      spdlog::error("Failed to get actual window pixel size: {}", SDL_GetError());
      actual_width = new_width;
      actual_height = new_height;
    }

    setLogicalSize(actual_width, actual_height);

    // TODO Additional handling after window resize
  }

  void SimulacrumEngine::onWindowEvent(const SDL_Event& event)
  {
    // TODO
  }

  void SimulacrumEngine::onDisplayChange(const SDL_Event& event)
  {
    // TODO
  }

} // namespace Simulacrum
