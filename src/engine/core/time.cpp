#include "time.hpp"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_timer.h>
#include <algorithm>

namespace engine::core {

    Time::Time() {}

    void Time::update() {
        frame_start_time_ = SDL_GetTicksNS();
        auto current_delta_time = static_cast<double>(frame_start_time_ - last_time_) / 1000000000.0;

        if (target_frame_time_ > 0.0) {
            limitFrameRate(current_delta_time);
        }

        else {
            delta_time_ = current_delta_time;
        }

        last_time_ = SDL_GetTicksNS();
    }

    void Time::limitFrameRate(float current_delta_time) {
        if (current_delta_time < target_frame_time_) {
            double time_to_wait = target_frame_time_ - current_delta_time;
            Uint64 ns_to_wait = static_cast<Uint64>(time_to_wait * 1000000000.0);
            SDL_DelayNS(ns_to_wait);
            delta_time_ = static_cast<double>(SDL_GetTicksNS() - last_time_) / 1000000000.0;
        }

        else {
            delta_time_ = std::min(static_cast<double>(current_delta_time), 0.1);
        }
    }

    float Time::getDeltaTime() const {
        return delta_time_ * time_scale_;
    }

    float Time::getUnscaledDeltaTime() const {
        return delta_time_;
    }

    void Time::setTimeScale(float scale) {
        if (scale < 0.0) {
            spdlog::warn("Time scale can't be negative.; Clamping to 0.");
            scale = 0.0;
        }

        time_scale_ = scale;
    }

    float Time::getTimeScale() const {
        return time_scale_;
    }

    void Time::setTargetFps(int fps) {
        if (fps < 0) {
            spdlog::warn("Target FPS can't be negative. Setting to 0 (unlimited).");
            target_fps_ = 0;
        }

        else {
            target_fps_ = fps;
        }

        if (target_fps_ > 0) {
            target_frame_time_ = 1.0 / static_cast<double>(target_fps_);
            spdlog::info("Target FPS set to: {} (Frame time: {:.6f}s).", target_fps_, target_frame_time_);
        }

        else {
            target_frame_time_ = 0.0;
            spdlog::info("Target FPS set to: Unlimited.");
        }
    }

    int Time::getTargetFps() const {
        return target_fps_;
    }

} // namespace engine::core
