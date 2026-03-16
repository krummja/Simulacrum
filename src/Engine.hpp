#ifndef SIMULACRUM_ENGINE_HPP_
#define SIMULACRUM_ENGINE_HPP_

#include <memory>
#include <string_view>

#include "TimestepManager.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_video.h"

namespace Simulacrum
{
  class Engine
  {
  public:

    ~Engine() = default;

    static Engine& Instance()
    {
      static Engine instance;
      return instance;
    }


    // Engine API

    bool init(std::string_view title);

    void handleEvents();

    void update(float delta_time);

    void render();

    void present();

    void clean();

    void toggleFullscreen();

    void setFullscreen(bool enabled);

    void setGlobalPause(bool paused);


    // Getters & Setters

    bool isRunning() const { return is_running_; }

    void setIsRunning(const bool is_running) { is_running_ = is_running; }

    bool isFullscreen() const noexcept { return is_fullscreen_; }

    bool isVsyncEnabled() const noexcept { return is_vsync_requested_; }

    bool isGlobalPaused() const noexcept { return is_global_paused_; }

    int windowWidth() const { return window_width_; }

    int windowHeight() const { return window_height_; }

    int windowedWidth() const { return windowed_width_; }

    int windowedHeight() const { return windowed_height_; }

    void setWindowSize(const int width, const int height)
    {
      window_width_ = width;
      window_height_ = height;

      if (!is_fullscreen_)
      {
        windowed_width_ = width;
        windowed_height_ = height;
      }
    }

    void setLogicalSize(const int width, const int height)
    {
      logical_width_ = width;
      logical_height_ = height;
    }

    SDL_Window* getWindow() const noexcept
    {
      return window_.get();
    }

  private:
    Engine()
      : window_width_{ 1280 }
      , window_height_{ 720 }
    {}

    // No-copy
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool is_running_{ false };
    bool is_fullscreen_{ false };
    bool is_window_occluded_{ false };
    bool is_vsync_requested_{ false };
    bool is_global_paused_{ false };

    int window_width_{ 0 };
    int window_height_{ 0 };
    int windowed_width_{ 0 };
    int windowed_height_{ 0 };
    int logical_width_{ 0 };
    int logical_height_{ 0 };
    float dpi_scale_{ 1.0f };

    std::unique_ptr<TimestepManager> timestep_manager_{ nullptr };
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_{ nullptr, SDL_DestroyWindow };

    void onWindowResize(const SDL_Event& event);

    void onWindowEvent(const SDL_Event& event);

    void onDisplayChange(const SDL_Event& event);
  };
}

#endif