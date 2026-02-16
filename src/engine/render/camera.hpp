#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "../utils/math.hpp"
#include <optional>

namespace engine::object::components {
    class TransformComponent;
}

namespace engine::render {

    class Camera final {
    public:
        Camera(
            glm::vec2 viewport_size,
            glm::vec2 position = glm::vec2(0.0f, 0.0f),
            std::optional<engine::utils::Rect> limit_bounds = std::nullopt
        );

        void update(float delta_time);
        void move(const glm::vec2& offset);

        glm::vec2 worldToScreen(const glm::vec2& world_pos) const;
        glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const;
        glm::vec2 screenToWorld(const glm::vec2& screen_pos) const;

        const glm::vec2& getPosition() const;
        std::optional<engine::utils::Rect> getLimitBounds() const;
        glm::vec2 getViewportSize() const;
        engine::object::components::TransformComponent* getTarget() const;

        void setPosition(glm::vec2 position);
        void setLimitBounds(std::optional<engine::utils::Rect> limit_bounds);
        void setTarget(engine::object::components::TransformComponent* target);

        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;
        Camera(Camera&&) = delete;
        Camera& operator=(Camera&&) = delete;

    private:
        glm::vec2 viewport_size_;
        glm::vec2 position_;
        std::optional<engine::utils::Rect> limit_bounds_;
        float smooth_speed_ = 5.0f;
        engine::object::components::TransformComponent* target_ = nullptr;

        void clampPosition();
    };

} // namespace engine::render

#endif // CAMERA_HPP_