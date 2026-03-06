#pragma once

#include "macros.hpp"

#include <cstdint>
#include <chrono>
#include <SDL3/SDL.h>

namespace Simulacrum
{

  /// @brief TimestepManager provides consistent game timing with simplified accumulator
  /// handling.
  ///
  /// This separates update timing (fixed timestep for consistent physics/logic) from
  /// render timing (variable timestep for smooth visuals).
  ///
  /// Uses 1:1 frame-to-update mapping to eliminate timing drift and micro-stuttering that
  /// can occur with traditional accumulator patterns.
  ///
  class TimestepManager
  {
  public:

    /// @brief Constructor
    /// @param target_fps Target frames per second for rendering (e.g. 60.0f)
    /// @param fixed_timestep Fixed timestep for updates in seconds (e.g. 1.0f/60.0f)
    explicit TimestepManager(float target_fps = 60.0f, float fixed_timestep = 1.0f / 60.0f);

    /// @brief Call this at the start of each frame.
    void startFrame();

    /// @brief Returns true if an update should be performed with fixed timestep.
    /// May return true multiple times per frame for catch-up.
    bool shouldUpdate();

    /// @brief Returns true if rendering should be peformed, typically once per frame.
    bool shouldRender() const;

    /// @brief Gets the fixed delta time for updates.
    /// Always returns the same value for consistent physics.
    /// @return Fixed timestep in seconds
    float getUpdateDeltaTime() const;

    /// @brief Gets the interpolation factor (alpha) for smooth rendering between fixed
    /// updates. This represents the fraction of time into the next fixed update step.
    /// @return Interpolation alpha value (typically between 0.0 and 1.0)
    double getInterpolationAlpha() const;

    /// @brief Call this at the end of each frame.
    /// Handles frame rate limiting via sleep/delay.
    void endFrame();

    /// @brief Get current measured FPS.
    /// @return Current frames per second
    float getCurrentFPS() const;

    /// @brief Get target FPS.
    /// @return Target frames per second
    float getTargetFPS() const;

    /// @brief Get last frame time in milliseconds.
    /// @return Frame time in milliseconds.
    uint32_t getFrameTimeMs() const;

    /// @brief Check if the last frame time exceeded target time significantly.
    /// @return True if frame tiem was excessive
    bool isFrameTimeExcessive() const;

    /// @brief Set new target FPS (updates frame time target).
    /// @param fps New target frames per second
    void setTargetFPS(float fps);

    /// @brief Set new fixed timestep for updates.
    /// @param timestep New fixed timestep in seconds
    void setFixedTimestep(float timestep);

    /// @brief Get update frequency in Hz (inverse of fixed timestep).
    /// @return Update frequency in Hz (e.g. 60.0 for 60 Hz updates)
    float getUpdateFrequencyHz() const { return 1.0f / fixed_timestep_; }

    /// @brief Reset timing state (useful for pausing/unpausing).
    void reset();

    void setSoftwareFrameLimiting(bool use_software_limiting) const;

    bool isUsingSoftwareFrameLimiting() const { return using_software_frame_limiting_; }

    void preciseFrameWait(double target_frame_time_ms) const;

  private:
    float target_fps_;
    float fixed_timestep_;
    float target_frame_time_;

    std::chrono::high_resolution_clock::time_point frame_start_;
    std::chrono::high_resolution_clock::time_point last_frame_time_;

    double accumulator_;
    static constexpr double MAX_ACCUMULATOR = 0.25;

    uint32_t last_frame_time_ms_;
    double last_delta_seconds_;
    float current_fps_;
    float smoothing_alpha_;

    bool should_render_;
    bool first_frame_;

    mutable bool using_software_frame_limiting_ = false;
    mutable bool explicitly_set_ = false;

    void updateFPS();
    void limitFrameRate() const;
  };

} // namespace Simulacrum
