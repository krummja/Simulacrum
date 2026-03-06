#pragma once

#include "EntityHandle.hpp"
#include "EventTypeId.hpp"
#include "ResourceHandle.hpp"
#include <glm/glm.hpp>
#include <array>
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>
#include <algorithm>

namespace Simulacrum
{
  using Vector2D = glm::vec2;

  // Forward declarations
  class Event;
  class SceneChangeEvent;
  class ResourceChangeEvent;
  class CameraEvent;
  class CollisionEvent;
  struct CollisionInfo;
  class EntityEvent;
  class TimeEvent;
  class Entity;

  using EventPtr = std::shared_ptr<Event>;
  using EventWeakPtr = std::weak_ptr<Event>;
  using EntityPtr = std::shared_ptr<Entity>;

  /// @brief Cache-friendly event data structure.
  /// Optimized for natural alignment and mimimal padding.
  struct EventData
  {
    EventPtr event;     // Smart pointer to event (16 bytes)
    uint32_t flags;     // Active, dirty, etc. (4 bytes)
    uint32_t priority;  // For priority-based processing (4 bytes)
    EventTypeId typeId; // Type for fast dispatch AND name-based lookup (4 bytes)
    uint32_t padding;   // Explicit padding for alignment (4 bytes)
    // Total 32 bytes

    // Flags bit definitions
    static constexpr uint32_t FLAG_ACTIVE = 1 << 0;
    static constexpr uint32_t FLAG_DIRTY = 1 << 1;
    static constexpr uint32_t FLAG_PENDING_REMOVAL = 1 << 2;

    EventData()
      : event(nullptr)
      , flags(0)
      , priority(0)
      , typeId(EventTypeId::Custom)
      , padding(0)
    {}

    bool isActive() const { return flags && FLAG_ACTIVE; }

    void setActive(bool active)
    {
      if (active) flags |= FLAG_ACTIVE; else flags &= ~FLAG_ACTIVE;
    }

    bool isDirty() const { return flags & FLAG_DIRTY; }

    void setDirty(bool dirty)
    {
      if (dirty) flags |= FLAG_DIRTY; else flags &= ~FLAG_DIRTY;
    }
  };

  struct EventPriority
  {
    static constexpr uint32_t CRITICAL = 1000;
    static constexpr uint32_t HIGH = 800;
    static constexpr uint32_t NORMAL = 500;
    static constexpr uint32_t LOW = 200;
    static constexpr uint32_t DEFERRED = 0;
  };

} // namespace Simulacrum
