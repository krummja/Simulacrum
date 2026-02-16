#include "transform_component.hpp"
#include "../game_object.hpp"

namespace engine::object::components {

    void TransformComponent::setScale(glm::vec2 scale) {
        scale_ = std::move(scale);
    }

} // namespace engine::object::components
