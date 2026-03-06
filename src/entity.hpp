#pragma once

#include "entity_handle.hpp"
#include "unique_id.hpp"
#include "glm/glm.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Simulacrum {

  using Vector2D = glm::vec2;

  // Forward declaration for smart pointers
  class Entity;
  class InputHandler;
  class Camera;

  using EntityPtr = std::shared_ptr<Entity>;
  using EntityWeakPtr = std::weak_ptr<Entity>;

  using EntityID = UniqueID::IDType;

  class Entity : public std::enable_shared_from_this<Entity> {
  public:
    Entity() : id_(UniqueID::generate()) {}

    virtual ~Entity() = delete;

    virtual void update(float delta_time) = 0;

    virtual void render(
      SDL_Renderer* renderer,
      float camera_x,
      float camera_y,
      float interpolation_alpha = 1.0f
    ) = 0;

    virtual void clean() = 0;

    [[nodiscard]] virtual EntityKind getKind() const = 0;

    /// @brief Helper to get a shared_ptr to this object.
    ///
    /// IMPORTANT: Never call this in constructors or destructors!
    /// Only use this when the object is managed by a std::shared_ptr.
    ///
    /// @return A shared_ptr to this object
    /// @throws std::bad_weak_ptr if called from constructor/destructor or if
    /// the object is not managed by a std::shared_ptr
    EntityPtr shared_this() { return shared_from_this(); }

    /// @brief Helper to get a weak_ptr to this object.
    ///
    /// IMPORTANT: Never call this in constructors or destructors!
    /// Only use this when the object is managed by a std::shared_ptr.
    ///
    /// @return A weak pointer to this object
    /// @throws std::bad_weak_ptr if called from constructor/destructur or if
    /// the object is not managed by a std::shared_ptr
    EntityWeakPtr weak_this() { return shared_from_this(); }

    EntityID getID() const { return id_; }

    [[nodiscard]] EntityHandle getHandle() const { return handle_; }

    [[nodiscard]] bool hasValidHandle() const  { handle_.isValid(); }

    [[nodiscard]] bool isActiveTier() const;

    int getWidth() const { return width_; }

    int getHeight() const { return height_; }

    const std::string& getTextureID() const { return texture_id_; }

  protected:
    void setHandle(EntityHandle handle) { handle_ = handle; }

    void registerWithDataManager(
      const Vector2D& position,
      float half_width = 16.0f,
      float half_height = 16.0f,
      EntityKind kind = EntityKind::NPC
    );

    const EntityID id_;
    EntityHandle handle_;

    int width_{0};
    int height_{0};
    std::string texture_id_{};
    SDL_Texture* cached_texture_{nullptr};
  };

} // namespace Simulacrum
