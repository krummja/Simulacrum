#pragma once

#include "macros.hpp"
#include "timestep_manager.hpp"
#include "state_manager.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <string_view>

namespace Simulacrum
{
  // Forward declarations
  class GPURenderer;

  class SimulacrumEngine
  {
  public:
    ~SimulacrumEngine() = default;

    static SimulacrumEngine& Instance()
    {
      static SimulacrumEngine instance;
      return instance;
    }

    bool init(std::string_view title);

    void handleEvents();

    void update(float deltaTime);

    void render();

    void present();

    void clean();

    void processBackgroundTasks();

    StateManager* getStateManager() const
    {
      return state_manager_.get();
    }

    TimestepManager& getTimestepManager()
    {
      return *timestep_manager_;
    }

    const TimestepManager& getTimestepManager() const
    {
      return *timestep_manager_;
    }

    bool isRunning() const { return running_; }

    void stop() { running_ = false; }

    void setRunning(bool running);

    // SDL_Renderer* getRenderer() const noexcept {
    //     return renderer_.get();
    // }

    SDL_Window* getWindow() const noexcept
    {
      return window_.get();
    }

    float getCurrentFPS() const;

    int getWindowWidth() const noexcept { return window_width_; }

    int getWindowHeight() const noexcept { return window_height_; }

    int getLogicalWidth() const noexcept { return logical_width_; }

    int getLogicalHeight() const noexcept { return logical_height_; }

    void setWindowSize(int width, int height)
    {
      window_width_ = width;
      window_height_ = height;

      if (!is_fullscreen_)
      {
        windowed_width_ = width;
        windowed_height_ = height;
      }
    }

    void setLogicalSize(int width, int height)
    {
      logical_width_ = width;
      logical_height_ = height;
    }

    void setLogicalPresentationMode(SDL_RendererLogicalPresentation mode);

    SDL_RendererLogicalPresentation getLogicalPresentationMode() const noexcept;

    float getDPIScale() const { return dpi_scale_; }

    void setDPIScale(float new_scale) { dpi_scale_ = new_scale; }

    bool isVSyncEnabled() const noexcept;

    bool setVSyncEnabled(bool enable);

    void toggleFullscreen();

    void setFullscreen(bool enabled);

    bool isFullscreen() const noexcept { return is_fullscreen_; }

    void setGlobalPause(bool paused);

    bool isGlobalPaused() const;

  private:
    bool verifyVsyncState(bool requested);
    void onWindowResize(const SDL_Event& event);
    void onWindowEvent(const SDL_Event& event);
    void onDisplayChange(const SDL_Event& event);

    std::unique_ptr<StateManager> state_manager_{ nullptr };
    std::unique_ptr<TimestepManager> timestep_manager_{ nullptr };

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window_{ nullptr, SDL_DestroyWindow };
    // std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer_{nullptr, SDL_DestroyRenderer};

    bool running_{ false };
    int window_width_{ 0 };
    int window_height_{ 0 };
    int windowed_width_{ 0 };
    int windowed_height_{ 0 };
    int logical_width_{ 0 };
    int logical_height_{ 0 };

    SDL_RendererLogicalPresentation logical_presentation_mode_{ SDL_LOGICAL_PRESENTATION_LETTERBOX };

    float dpi_scale_{ 1.0f };

    bool is_fullscreen_{ false };
    bool vsync_requested_{ true };
    bool window_occluded_{ false };

    bool globally_paused_{ false };

    SimulacrumEngine(const SimulacrumEngine&) = delete;
    SimulacrumEngine& operator=(const SimulacrumEngine&) = delete;

    SimulacrumEngine() : window_width_{ 1280 }, window_height_{ 720 } {}
  };

} // namespace Simulacrum
