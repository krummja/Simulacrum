#pragma once

#include "glm/glm.hpp"
#include <algorithm>
#include <memory>
#include <functional>
#include <cstdint>
#include <random>

using Vector2D = glm::vec2;

namespace Simulacrum
{
    // Forward declaration
    class Entity;

    class Camera
    {
    public:

        enum class Mode { Free, Follow, Fixed };

        struct Config
        {
            float follow_speed{5.0f};
            float deadzone_radius{32.0f};
            float max_follow_distance{200.0f};
            float smoothing_factor{0.85};
            bool clamp_to_world_bounds{true};

            std::vector<float> zoom_levels{1.0f, 2.0f, 3.0f};
            int default_zoom_level{0};

            bool isValid() const {
                if (follow_speed <= 0.0f || deadzone_radius < 0.0f || max_follow_distance <= 0.0f)
                {
                    return false;
                }

                if (smoothing_factor < 0.0f || smoothing_factor > 1.0f)
                {
                    return false;
                }

                if (zoom_levels.empty())
                {
                    return false;
                }


                bool zoom_condition = std::all_of(
                    zoom_levels.begin(), zoom_levels.end(),
                    [](float zoom) { return zoom > 0.0f; }
                );

                if (!zoom_condition)
                {
                    return false;
                }

                if (default_zoom_level < 0 || default_zoom_level >= static_cast<int>(zoom_levels.size()))
                {
                    return false;
                }

                return true;
            }
        };

        struct Bounds
        {
            float min_x{0.0f};
            float min_y{0.0f};
            float max_x{1000.0f};
            float max_y{1000.0f};

            bool isValid() const
            {
                return max_x > min_x && max_y > min_y;
            }
        };

        struct Viewport
        {
            float width{0.0f};
            float height{0.0f};

            bool isValid() const
            {
                return width > 0.0f && height > 0.0f;
            }

            float halfWidth() const { return width * 0.5f; }
            float halfHeight() const { return height * 0.5f; }
        };


        Camera();

        explicit Camera(const Config& config);

        Camera(float x, float y, float viewport_width, float viewport_height);

        ~Camera() = default;

        // Copy operations
        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;

        // Move operations
        Camera(Camera&&) = delete;
        Camera& operator=(Camera&&) = delete;

        void update(float delta_time);

        void setPosition(float x, float y);

        void setPosition(const Vector2D& position);

        const Vector2D& getPosition() const { return position_; }

        float getX() const { return position_.x; }

        float getY() const { return position_.y; }

        void setViewport(float width, float height);

        void setViewport(const Viewport& viewport);

        const Viewport& getViewport() const { return viewport_; }

        void setWorldBounds(float min_x, float min_y, float max_x, float max_y);

        void setWorldBounds(const Bounds& bounds);

        const Bounds& getWorldBounds() const { return world_bounds_; }

        void setMode(Mode mode);

        Mode getMode() const { return mode_; }

        void setTarget(const std::weak_ptr<Entity>& target);

        void setTargetPositionGetter(std::function<Vector2D()> position_getter);

        void clearTarget();

        bool hasTarget() const;

        bool setConfig(const Config& config);

        const Config& getConfig() const { return config_; }

        struct ViewRect
        {
            float x;
            float y;
            float width;
            float height;

            float left() const { return x; }
            float right() const { return x + width; }
            float top() const { return y; }
            float bottom() const { return y + height; }
            float centerX() const { return x + width * 0.5f; }
            float centerY() const { return y + height * 0.5f; }
        };

        ViewRect getViewRect() const;

        Vector2D getRenderOffset(float& offset_x, float& offset_y, float interpolation_alpha = 1.0f) const;

        bool isPointVisible(float x, float y) const;

        bool isPointVisible(const Vector2D& point) const;

        bool isRectVisible(float x, float y, float width, float height) const;

        void worldToScreen(float wx, float wy, float& sx, float& sy) const;

        void screenToWorld(float sx, float sy, float& wx, float& wy) const;

        Vector2D screenToWorld(const Vector2D& screen_coords) const;

        Vector2D worldToScreen(const Vector2D& world_Coords) const;

        void snapToTarget();

        void zoomIn();

        void zoomOut();

        void setZoomLevel(int level_index);

        float getZoom() const { return zoom_; }

        float getZoomLevel() const { return current_zoom_index_; }

        int getNumZoomLevels() const { return static_cast<int>(config_.zoom_levels.size()); }

        void syncViewportWithEngine();

    private:
        Vector2D position_{0.0f, 0.0f};
        Vector2D target_position_{0.0f, 0.0f};
        Viewport viewport_{};
        Bounds world_bounds_{};
        Config config_{};
        Mode mode_{Mode::Free};

        std::weak_ptr<Entity> target_;
        std::function<Vector2D()> position_getter_;

        float zoom_{1.0f};
        int current_zoom_index_{0};

        void syncWorldBounds();
        void clampToWorldBounds();
        Vector2D getTargetPosition() const;

        void computeOffsetFromCenter(float cx, float cy, float& offset_x, float& offset_y) const;
    };

} // namespace Simulacrum
