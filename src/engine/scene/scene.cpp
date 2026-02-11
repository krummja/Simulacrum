#include "scene.hpp"
#include "scene_manager.hpp"
// #include "../object/game_object.hpp"
#include "../core/context.hpp"
// #include "../core/game_state.hpp"
// #include "../physics/physics_engine.hpp"
// #include "../render/camera.hpp"
// #include "../ui/ui_manager.hpp"
#include <algorithm>  // for std::remove_if
#include <spdlog/spdlog.h>

namespace engine::scene {

    Scene::Scene(
        std::string_view name,
        engine::core::Context& context,
        engine::scene::SceneManager& scene_manager
    )
        : scene_name_(name)
        , context_(context)
        , scene_manager_(scene_manager)
        , ui_manager_(std::make_unique<engine::ui::UIManager>())
        , is_initialized_(false)
    {}

    Scene::~Scene() = default;

    void Scene::init() {}

    void Scene::update(float delta_time) {}

    void Scene::render() {}

    void Scene::handleInput() {}

    void Scene::clean() {}

    void Scene::addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object) {}

    void Scene::safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object) {}

    void Scene::removeGameObject(engine::object::GameObject* game_object_ptr) {}

    void Scene::safeRemoveGameObject(engine::object::GameObject* game_object_ptr) {}

    const engine::object::GameObject* Scene::findGameObjectByName(std::string_view name) const {
        // for (const auto& obj : game_objects_) {
        //     if (obj && obj->getName() == name) {
        //         return obj.get();
        //     }
        // }
        return nullptr;
    }

    void Scene::processPendingAdditions() {}
}
