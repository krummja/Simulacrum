#include "TimestepManager.hpp"
#include <algorithm>
#include <chrono>

namespace Simulacrum
{

  TimestepManager::TimestepManager(float target_fps, float fixed_timestep)
    : target_fps_(target_fps)
    , fixed_timestep_(fixed_timestep)
    , target_frame_time_(1.0f / target_fps)
    , accumulator_(0.0)
    , last_frame_time_ms_(0)
    , last_delta_seconds_(0.0)
    , current_fps_(0.0f)
    , smoothing_alpha_(0.03f)
    , should_render_(true)
    , first_frame_(true)
  {
    auto current_time = std::chrono::high_resolution_clock::now();
    frame_start_ = current_time;
    last_frame_time_ = current_time;

    using_software_frame_limiting_ = false;
    explicitly_set_ = false;
  }

  void TimestepManager::startFrame()
  {
    auto current_time = std::chrono::high_resolution_clock::now();

    if (first_frame_)
    {
      first_frame_ = false;
      last_frame_time_ = current_time;
      frame_start_ = current_time;
      return;
    }

    // Calculate frame delta time in seconds
    auto delta_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_frame_time_);
    double delta_time_ms = static_cast<double>(delta_time_ns.count()) / 1000000.0;
    last_frame_time_ = current_time;
    frame_start_ = current_time;

    // Update frame time in milliseconds (for getFrameTimeMs() API)
    last_frame_time_ms_ = static_cast<uint32_t>(delta_time_ms);

    // Convert to seconds
    double delta_time = delta_time_ms / 1000.0;
    last_delta_seconds_ = delta_time;

    // Mode-aware accumulator handling:
    // VSync mode: Normal accumulator pattern - add delta (clamped to prevent spiral)
    //             Allows catch-up updates under load, uses interpolation for smoothness
    // Software mode: Force exactly one update per frame by setting accumulator = timestep
    //                Since we control timing via SDL_DelayPrecise, no catch-up needed
    //                This eliminates timing jitter causing 0-update or 2-update frames
    if (using_software_frame_limiting_)
    {
      // Set to exactly one timestep - guarantees one update, alpha = 0 after
      accumulator_ = fixed_timestep_;
    }
    else
    {
      delta_time = std::min(delta_time, MAX_ACCUMULATOR);
      accumulator_ += delta_time;
    }

    // Always render once per frame
    should_render_ = true;

    // Update FPS counter
    updateFPS();
  }

  bool TimestepManager::shouldUpdate()
  {
    if (accumulator_ >= fixed_timestep_)
    {
      accumulator_ -= fixed_timestep_;
      return true;
    }
    return false;
  }

  bool TimestepManager::shouldRender() const
  {
    return should_render_;
  }

  float TimestepManager::getUpdateDeltaTime() const
  {
    return fixed_timestep_;
  }

  double TimestepManager::getInterpolationAlpha() const
  {
    if (fixed_timestep_ > 0.0f)
    {
      double alpha = accumulator_ / fixed_timestep_;
      return std::clamp(alpha, 0.0, 1.0);
    }
    return 1.0;
  }

  void TimestepManager::endFrame()
  {
    should_render_ = false;
    limitFrameRate();
  }

  float TimestepManager::getCurrentFPS() const
  {
    return current_fps_;
  }

  float TimestepManager::getTargetFPS() const
  {
    return target_fps_;
  }

  uint32_t TimestepManager::getFrameTimeMs() const
  {
    return last_frame_time_ms_;
  }

  void TimestepManager::setTargetFPS(float fps)
  {
    if (fps > 0.0f)
    {
      target_fps_ = fps;
      target_frame_time_ = 1.0f / fps;
    }
  }

  void TimestepManager::setFixedTimestep(float timestep)
  {
    if (timestep > 0.0f)
    {
      fixed_timestep_ = timestep;
    }
  }

  void TimestepManager::reset()
  {
    accumulator_ = 0.0;
    first_frame_ = true;
    should_render_ = true;
    current_fps_ = 0.0f;
    last_delta_seconds_ = 0.0;

    auto current_time = std::chrono::high_resolution_clock::now();
    frame_start_ = current_time;
    last_frame_time_ = current_time;

    if (explicitly_set_)
    {
      using_software_frame_limiting_ = false;
    }
  }

  void TimestepManager::updateFPS()
  {
    if (last_delta_seconds_ > 0.0)
    {
      float instant_fps = static_cast<float>(1.0 / last_delta_seconds_);
      instant_fps = std::clamp(instant_fps, 0.1f, 1000.0f);

      if (current_fps_ <= 0.0f)
      {
        current_fps_ = instant_fps;
      }
      else
      {
        current_fps_ = smoothing_alpha_ * instant_fps + (1.0f - smoothing_alpha_) * current_fps_;
      }
    }
  }

  void TimestepManager::limitFrameRate() const
  {
    // If using hardware VSync, skip software limiting - VSync handles timing via
    // SDL_RenderPresent()
    if (!using_software_frame_limiting_)
    {
      return;
    }

    // Calculate absolute target end time for this frame
    // Using nanoseconds for maximum precision
    int64_t target_frame_ns = static_cast<int64_t>(target_frame_time_ * 1e9);
    auto target_end_time = frame_start_ + std::chrono::nanoseconds(target_frame_ns);

    // Calculate remaining time NOW (minimizes overhead between calculation and delay)
    auto now = std::chrono::high_resolution_clock::now();
    auto remaining_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(target_end_time - now);

    // Only delay if we have time remaining
    if (remaining_ns.count() > 0)
    {
      SDL_DelayPrecise(static_cast<Uint64>(remaining_ns.count()));
    }
  }

  void TimestepManager::setSoftwareFrameLimiting(bool use_software_limiting) const
  {
    using_software_frame_limiting_ = use_software_limiting;
    explicitly_set_ = true;
  }

  void TimestepManager::preciseFrameWait(double target_frame_time_ms) const
  {
    Uint64 target_ns = static_cast<Uint64>(target_frame_time_ms * 1000000.0);
    SDL_DelayPrecise(target_ns);
  }

} // namespace Simulacrum
