#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

float* matToFloatVec(glm::mat4 matrix)
{
    const float* out = glm::value_ptr(matrix);
    float new_array[16];
    memcpy(new_array, out, sizeof(new_array));
    return new_array;
}
