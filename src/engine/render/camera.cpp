#include "camera.hpp"
#include "../utils/math.hpp"
#include "../object/components/transform_component.hpp"
#include <spdlog/spdlog.h>

namespace engine::render {

    Camera::Camera(
        glm::vec2 viewport_size,
        glm::vec2 position,
        std::optional<engine::utils::Rect> limit_bounds
    )
        : viewport_size_(std::move(viewport_size))
        , position_(std::move(position))
        , limit_bounds_(std::move(limit_bounds))
    {}

    void Camera::setPosition(glm::vec2 position) {
        position_ = std::move(position);
        clampPosition();
    }

    void Camera::update(float delta_time) {
        if (target_ == nullptr) {
            return;
        }

        glm::vec2 target_pos = target_->getPosition();
        glm::vec2 desired_position = target_pos - viewport_size_ / 2.0f;

        auto distance_ = glm::distance(position_, desired_position);
        constexpr float SNAP_THRESHOLD = 1.0f;

        if (distance_ < SNAP_THRESHOLD) {
            position_ = desired_position;
        } else {
            position_ = glm::mix(position_, desired_position, smooth_speed_ * delta_time);
            position_ = glm::vec2(glm::round(position_.x), glm::round(position_.y));
        }

        clampPosition();
    }

    void Camera::move(const glm::vec2& offset) {
        position_ += offset;
        clampPosition();
    }

    void Camera::setLimitBounds(std::optional<engine::utils::Rect> limit_bounds) {
        limit_bounds_ = std::move(limit_bounds);
        clampPosition();
    }

    void Camera::setTarget(engine::object::components::TransformComponent *target) {
        target_ = target;
    }

    engine::object::components::TransformComponent* Camera::getTarget() const {
        return target_;
    }

    const glm::vec2& Camera::getPosition() const {
        return position_;
    }

    void Camera::clampPosition() {
        if (limit_bounds_.has_value() && limit_bounds_->size.x > 0 && limit_bounds_->size.y > 0) {
            glm::vec2 min_cam_pos = limit_bounds_->position;
            glm::vec2 max_cam_pos = limit_bounds_->position + limit_bounds_->size - viewport_size_;

            max_cam_pos.x = std::max(min_cam_pos.x, max_cam_pos.x);
            max_cam_pos.y = std::max(min_cam_pos.y, max_cam_pos.y);

            position_ = glm::clamp(position_, min_cam_pos, max_cam_pos);
        }
    }

    glm::vec2 Camera::worldToScreen(const glm::vec2& world_pos) const {
        return world_pos - position_;
    }

    glm::vec2 Camera::worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const {
        return world_pos - position_ * scroll_factor;
    }

    glm::vec2 Camera::screenToWorld(const glm::vec2& screen_pos) const {
        return screen_pos + position_;
    }

    glm::vec2 Camera::getViewportSize() const {
        return viewport_size_;
    }

    std::optional<engine::utils::Rect> Camera::getLimitBounds() const {
        return limit_bounds_;
    }

} // namespace egine::render
