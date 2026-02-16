#include "game_object.hpp"
#include "../render/renderer.hpp"
#include "../input/input_manager.hpp"
#include "../render/camera.hpp"
#include <spdlog/spdlog.h>

namespace engine::object {

    GameObject::GameObject(
        std::string_view name,
        std::string_view tag
    )
        : name_(name)
        , tag_(tag)
    {
        spdlog::trace("GameObject created: {} {}", name_, tag_);
    }

    void GameObject::update(float delta_time, engine::core::Context& context) {
        for (auto& pair : components_) {
            pair.second->update(delta_time, context);
        }
    }

    void GameObject::render(engine::core::Context& context) {
        for (auto& pair : components_) {
            pair.second->render(context);
        }
    }

    void GameObject::clean() {
        for (auto& pair : components_) {
            pair.second->clean();
        }
        components_.clear();
    }

    void GameObject::handleInput(engine::core::Context& context) {
        for (auto& pair : components_) {
            pair.second->handleInput(context);
        }
    }

} // namespace engine::object
