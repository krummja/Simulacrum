#include "scene_manager.hpp"
#include "scene.hpp"
#include "../core/context.hpp"
#include <spdlog/spdlog.h>

namespace engine::scene {

    SceneManager::SceneManager(engine::core::Context& context) : context_(context) {}

    SceneManager::~SceneManager() {
        close();
    }

    Scene* SceneManager::getCurrentScene() const {
        if (scene_stack_.empty()) {
            return nullptr;
        }

        return scene_stack_.back().get();
    }

    void SceneManager::update(float delta_time) {
        Scene* current_scene = getCurrentScene();
        if (current_scene) {
            current_scene->update(delta_time);
        }

        processPendingActions();
    }

    void SceneManager::render() {
        for (const auto& scene : scene_stack_) {
            if (scene) {
                scene->render();
            }
        }
    }

    void SceneManager::handleInput() {
        Scene* current_scene = getCurrentScene();
        if (current_scene) {
            current_scene->handleInput();
        }
    }

    void SceneManager::close() {
        while(!scene_stack_.empty()) {
            if (scene_stack_.back()) {
                scene_stack_.back()->clean();
            }
            scene_stack_.pop_back();
        }
    }

    void SceneManager::requestPushScene(std::unique_ptr<Scene>&& scene) {
        pending_action_ = PendingAction::Push;
        pending_scene_ = std::move(scene);
    }

    void SceneManager::requestPopScene() {
        pending_action_ = PendingAction::Pop;
    }

    void SceneManager::requestReplaceScene(std::unique_ptr<Scene>&& scene) {
        pending_action_ = PendingAction::Replace;
        pending_scene_ = std::move(scene);
    }


    // --- Private API ---

    void SceneManager::processPendingActions() {
        if (pending_action_ == PendingAction::None) {
            return;
        }

        switch (pending_action_) {
            case PendingAction::Push:
                pushScene(std::move(pending_scene_));
                break;
            case PendingAction::Pop:
                popScene();
                break;
            case PendingAction::Replace:
                replaceScene(std::move(pending_scene_));
                break;
            default:
                break;
        }

        pending_action_ = PendingAction::None;
    }

    void SceneManager::pushScene(std::unique_ptr<Scene>&& scene) {
        if (!scene) {
            return;
        }

        if (!scene->isInitialized()) {
            scene->init();
        }

        scene_stack_.push_back(std::move(scene));
    }

    void SceneManager::popScene() {
        if (scene_stack_.empty()) {
            return;
        }

        if (scene_stack_.back()) {
            scene_stack_.back()->clean();
        }

        scene_stack_.pop_back();
    }

    void SceneManager::replaceScene(std::unique_ptr<Scene>&& scene) {
        if (!scene) {
            return;
        }

        while (!scene_stack_.empty()) {
            if (scene_stack_.back()) {
                scene_stack_.back()->clean();
            }

            scene_stack_.pop_back();
        }

        if (!scene->isInitialized()) {
            scene->init();
        }

        scene_stack_.push_back(std::move(scene));
    }
}
