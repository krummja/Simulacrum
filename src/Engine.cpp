#include "Engine.hpp"

#include <future>

#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>

#include "ResourcePath.hpp"
#include "SettingsManager.hpp"
#include "GpuDevice.hpp"
#include "GPURenderer.hpp"
#include "InputManager.hpp"
#include "ThreadSystem.hpp"

namespace Simulacrum
{
  bool Engine::init(const std::string_view title)
  {
    // Initialize SDL

    spdlog::info("Initializing SDL video and gamepad");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
      spdlog::error("SDL initialization failed: {}", SDL_GetError());
      return false;
    }

    spdlog::info("SDL video online");


    // Load Settings

    spdlog::info("Loading settings");
    const std::string settings_path = ResourcePath::resolve("res/settings.json");
    auto& settings_manager = SettingsManager::Instance();
    settings_manager.loadFromFile(settings_path);

    const auto& graphics_settings = settings_manager.getGraphicsSettings();

    const int width = graphics_settings.resolution_width;
    const int height = graphics_settings.resolution_height;
    const bool fullscreen = graphics_settings.fullscreen;

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


    // Initialize SDL Window

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


    // GPU Device

    if (auto& gpu_device = GPUDevice::Instance(); gpu_device.init(window_.get()))
    {
      if (auto& gpu_renderer = GPURenderer::Instance(); gpu_renderer.init())
      {
        spdlog::info("SDL3 GPU rendering initialized successfully");
      }
      else
      {
        spdlog::critical("Failed to initialize SDL3 GPU rendering");
        gpu_device.shutdown();
        return false;
      }
    }

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

    // Store actual dimensions for UI positioning
    int const actual_width = pixel_width;
    int const actual_height = pixel_height;
    logical_width = actual_width;
    logical_height = actual_height;

    spdlog::info("Using native resolution: {}x{}", actual_width, actual_height);

    // Calculate DPI-aware font sizes before threading
    dpi_scale_ = 1.0f;

    spdlog::info(
      "DPI scale: {}, window: {}x{}",
      dpi_scale_, window_width_, window_height_
    );

    // Unified VSync initialization with automatic fallback
    auto& settings = SettingsManager::Instance();
    const bool vsync_requested = settings.getGraphicsSettings().vsync;
    is_vsync_requested_ = vsync_requested;

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


    // Threaded Initialization Tasks

    std::vector<std::future<bool>> init_tasks;
    init_tasks.reserve(4);

    init_tasks.push_back(
      ThreadSystem::Instance().enqueueTaskWithResult(
        []() -> bool
        {
          if (InputManager& input_manager = InputManager::Instance(); !input_manager.init())
          {
            spdlog::critical("Failed to initialize InputManager");
            return false;
          }
          return true;
        }
      )
    );

    Instance().setIsRunning(true);
    return true;
  }

  void Engine::handleEvents()
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
          setIsRunning(false);
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
  }

  void Engine::update(float delta_time)
  {

  }

  void Engine::render()
  {

  }

  void Engine::present()
  {

  }

  void Engine::clean()
  {
    spdlog::info("Starting shutdown...");
    auto window_to_destroy = std::move(window_);

    spdlog::info("Shutting down GPU renderer...");
    GPURenderer::Instance().shutdown();

    spdlog::info("Shutting down GPU device...");
    GPUDevice::Instance().shutdown();

    spdlog::info("Destroying window...");
    window_to_destroy.reset();
    spdlog::info("Window destroyed successfully");

    spdlog::info("Shutting down SDL...");
    SDL_Quit();

    spdlog::info("Shutdown complete!");
  }

  void Engine::toggleFullscreen()
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

  void Engine::setFullscreen(const bool enabled)
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

  void Engine::onWindowResize(const SDL_Event& event)
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

  }

  void Engine::onWindowEvent(const SDL_Event& event)
  {
  }

  void Engine::onDisplayChange(const SDL_Event& event)
  {
  }

}
