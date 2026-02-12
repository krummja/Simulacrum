#ifndef TIME_HPP_
#define TIME_HPP_
#include <SDL3/SDL_stdinc.h>


namespace engine::core {

    class Time final {
    public:
        Time();

        Time(const Time&) = delete;
        Time& operator=(const Time&) = delete;
        Time(Time&&) = delete;
        Time& operator=(Time&&) = delete;

        /// @brief Called at the beginning of each frame, updates the internal time state
        /// and calculates delta time.
        void update();

        /// @brief Get the time difference between frames after time scaling adjustment.
        /// @return Scaled delta time (seconds).
        float getDeltaTime() const;

        /// @brief Get the original interframe time difference without time scaling.
        /// @return Unscaled delta time.
        float getUnscaledDeltaTime() const;

        /// @brief Set the time scaling factor
        /// @param scale Time scaling value (1.0 normal, <1.0 slow motion, >1.0 fast
        /// motion). Does not permit negative values.
        void setTimeScale(float scale);

        /// @brief Get the current time scaling factor.
        /// @return Current time scaling factor.
        float getTimeScale() const;

        /// @brief Sets the target frame rate.
        /// @param fps Target frames per second. Setting to 0 or below means unlimited.
        void setTargetFps(int fps);

        /// @brief Get the currently set target frame rate.
        /// @return Target FPS. 0 means unlimited.
        int getTargetFps() const;

    private:
        /// @brief Timestamp of the previous frame (used to calculate delta).
        Uint64 last_time_ = 0;

        /// @brief Timestamp of the start of the current frame (for fps limiting).
        Uint64 frame_start_time_ = 0;

        /// @brief Unscaled interframe time difference (seconds).
        double delta_time_ = 0.0;

        /// @brief Time scaling factor.
        double time_scale_ = 1.0;

        /// @brief Target FPS (0 = unlimited).
        int target_fps_ = 0;

        /// @brief Target time per frame (seconds).
        double target_frame_time_ = 0.0;

        /// @brief Called in update to limit the frame rate. If target_fps_ > 0 is set
        /// and the current frame execution time is less than the target frame time,
        /// SDL_DelayNS() is called to wait for the remaining time.
        ///
        /// @param current_delta_time Execution time of the current frame (seconds).
        void limitFrameRate(float current_delta_time);

    };

} // namespace engine::core

#endif // TIME_HPP_