#ifndef TRANSFORM_COMPONENT_HPP_
#define TRANSFORM_COMPONENT_HPP_

#include "../component.hpp"
#include <utility>
#include <glm/vec2.hpp>

namespace engine::object::components {

    class TransformComponent final : public Component {
        friend class engine::object::GameObject;

    public:
        glm::vec2 position_ = {0.0f, 0.0f};
        glm::vec2 scale_ = {1.0f, 1.0f};
        float rotation_ = 0.0f;

        TransformComponent(
            glm::vec2 position = {0.0f, 0.0f},
            glm::vec2 scale = {1.0f, 1.0f},
            float rotation = 0.0f
        )
            : position_(std::move(position))
            , scale_(std::move(scale))
            , rotation_(rotation)
        {}

        TransformComponent(const TransformComponent&) = delete;
        TransformComponent& operator=(const TransformComponent&) = delete;
        TransformComponent(TransformComponent&&) = delete;
        TransformComponent& operator=(TransformComponent&&) = delete;

        const glm::vec2& getPosition() const { return position_; }
        const glm::vec2& getScale() const { return scale_; }
        float getRotation() const { return rotation_; }
        void setPosition(glm::vec2 position) { position_ = std::move(position); }
        void setScale(glm::vec2 scale);
        void setRotation(float rotation) { rotation_ = rotation; }
        void translate(const glm::vec2 offset) { position_ += offset; }

    private:
        void update(float, engine::core::Context&) override {}

    };

} // namespace engine::object::components

#endif // TRANSFORM_COMPONENT_HPP_