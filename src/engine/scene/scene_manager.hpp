#ifndef SCENE_MANAGER_HPP_
#define SCENE_MANAGER_HPP_
#include <memory>
#include <string>
#include <vector>

namespace engine::core {
    class Context;
}

namespace engine::scene {
    class Scene;

    class SceneManager final {
    public:
        explicit SceneManager(engine::core::Context& context);
        ~SceneManager();

        SceneManager(const SceneManager&) = delete;
        SceneManager& operator=(const SceneManager&) = delete;
        SceneManager(SceneManager&&) = delete;
        SceneManager& operator=(SceneManager&&) = delete;

        void requestPushScene(std::unique_ptr<Scene>&& scene);
        void requestPopScene();
        void requestReplaceScene(std::unique_ptr<Scene>&& scene);

        Scene* getCurrentScene() const;
        engine::core::Context& getContext() const { return context_; }

        void update(float delta_time);
        void render();
        void handleInput();
        void close();

    private:
        engine::core::Context& context_;
        std::vector<std::unique_ptr<Scene>> scene_stack_;

        enum class PendingAction { None, Push, Pop, Replace };
        PendingAction pending_action_ = PendingAction::None;
        std::unique_ptr<Scene> pending_scene_;

        void processPendingActions();
        void pushScene(std::unique_ptr<Scene>&& scene);
        void popScene();
        void replaceScene(std::unique_ptr<Scene>&& scene);

    };
}

#endif // SCENE_MANAGER_HPP_
