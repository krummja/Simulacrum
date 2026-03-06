#pragma once

#include <cstdint>

namespace Simulacrum {

  enum class EventTypeId : uint8_t {
    SceneChange = 0,
    ResourceChange = 1,
    Camera = 2,
    Collision = 3,
    Entity = 4,
    Custom = 5,
    Time = 6,
    COUNT = 7
  };

} // namespace Simulacrum
