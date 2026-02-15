#ifndef MATH_HPP_
#define MATH_HPP_

#include <glm/glm.hpp>

namespace engine::utils {

    struct Rect {
        glm::vec2 position;
        glm::vec2 size;
    };

    struct FColor {
        float r;
        float g;
        float b;
        float a;
    };

} // namespace engine::utils

#endif // MATH_HPP_