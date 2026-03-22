#ifndef SIMULACRUM_FRAME_PROFILER_HPP_
#define SIMULACRUM_FRAME_PROFILER_HPP_

#include <array>
#include <chrono>
#include <cstdint>
#include <string>

struct SDL_Renderer;

namespace Simulacrum
{
  class FontManager;

  enum class FramePhase : uint8_t
  {
    Events = 0,
    Update,
    Render,
    PResent,
    COUNT
  };

  enum class ManagerPhase : uint8_t
  {
    Event = 0,
    GameState,
    AI,
    Particle,
    Collision,
    BackgroundSim,
    COUNT
  };

  enum class RenderPhase : uint8_t
  {
    BeginScene = 0,
    WorldTiles,
    Entities,
    EndScene,
    GPUCmdBuffer,
    GPUSwapchain,
    GPUVertexMap,
    GPUCopyPass,
    GPUUpload,
    GPUScenePass,
    GPUSwapPass,
    GPUSubmit,
    COUNT
  };

  class FrameProfiler
  {
  public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    static FrameProfiler& Instance();

    void setThresholdMs(double ms)
    {
      threshold_ms_ = ms;
    }

    double getThresholdMs() const
    {
      return threshold_ms_;
    }

    void toggleOverlay()
    {
      overlay_visible_ = !overlay_visible_;
    }

    bool isOverlayVisible() const
    {
      return overlay_visible_;
    }

    void suppressFrames(uint32_t frame_count = 5)
    {
      suppress_count_ = frame_count;
    }

    bool isSuppressed() const
    {
      return suppress_count_ > 0;
    }

    void beginFrame();

    void endFrame();

    void beginPhase(FramePhase phase);

    void endPhase(FramePhase phase);

    void beginManager(ManagerPhase phase);

    void endManager(ManagerPhase phase);

    void beginRender(RenderPhase phase);

    void endRender(RenderPhase phase);

    void renderOverlay(FontManager* font_manager);

    uint64_t getFrameCount() const
    {
      return frame_count_;
    }

    double getLastFrameTimeMs() const
    {
      return last_frame_time_ms_;
    }

  private:
    FrameProfiler() = default;
    ~FrameProfiler() = default;
    FrameProfiler(const FrameProfiler&) = delete;
    FrameProfiler& operator=(const FrameProfiler&) = delete;

    static const char* getPhaseName(FramePhase phase);
    static const char* getManagerName(ManagerPhase phase);
    static const char* getRenderPhaseName(RenderPhase phase);
    ManagerPhase findWorstManager() const;
    RenderPhase findWorstRenderPhase() const;

    // Timing data
    TimePoint frame_start_{};
    std::array<TimePoint, static_cast<size_t>(FramePhase::COUNT)> phase_starts_{};
    std::array<TimePoint, static_cast<size_t>(ManagerPhase::COUNT)> manager_starts_{};
    std::array<TimePoint, static_cast<size_t>(RenderPhase::COUNT)> render_starts_{};
    std::array<double, static_cast<size_t>(FramePhase::COUNT)> phase_times_{};
    std::array<double, static_cast<size_t>(ManagerPhase::COUNT)> manager_times_{};
    std::array<double, static_cast<size_t>(RenderPhase::COUNT)> render_times_{};

    // Configuration
    double threshold_ms_{20.0};
    uint32_t suppress_count_{0};

    // Statistics
    uint64_t frame_count_{0};
    uint64_t hitch_count_{0};
    double last_frame_time_ms_{0.0};

    // Last hitch info for overlay

    // Overlay state
    bool overlay_visible_{false};
    bool overlay_created_{false};

    // Text buffers for UI
    std::string frame_text_{};
    std::string update_text_{};
    std::string render_text_{};
    std::string present_text_{};
    std::string events_text_{};
    std::string threshold_text_{};
    std::string hitch_text_{};

    void createOverlayComponents();
    void destroyOverlayComponents();
    void updateOverlayText();
  };
}

#endif