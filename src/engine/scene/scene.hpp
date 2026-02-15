#ifndef SCENE_HPP_
#define SCENE_HPP_
#include <vector>
#include <memory>
#include <string>
#include <string_view>

namespace engine::core {
    class Context;
}

namespace engine::ui {
    class UIManager;
}

namespace engine::object {
    class GameObject;
}

namespace engine::scene {
    class SceneManager;

    class Scene {
    public:
        Scene(
            std::string_view name,
            engine::core::Context& context,
            engine::scene::SceneManager& scene_manager
        );

        /**
         * 1. The base class must declare a virtual destructor for the derived class
         *    destructor to be called correctly.
         * 2. The destructor definition must be written in cpp, otherwise it needs to be
         *    introduced into the GameObject header file.
         */
        virtual ~Scene();

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        virtual void init();                    ///< @brief Initialize the scene
        virtual void update(float delta_time);  ///< @brief Update scene
        virtual void render();                  ///< @brief Render the scene
        virtual void handleInput();             ///< @brief Process input
        virtual void clean();                   ///< @brief Clean up the scene

        /// @brief Add a GameObject directly to the scene. Available during initialization,
        /// unsafe in-game (&& indicates an rvalue reference, used with std::move to avoid
        /// copying).
        /// @param game_object
        virtual void addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object);

        /// @brief Safely add GameObject to the scene (adding to pending_additions_).
        /// @param game_object
        virtual void safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object);

        /// @brief Remove a GameObject directly from the scene (generally not used, but
        /// the implementation logic is retained).
        /// @param game_object_ptr
        virtual void removeGameObject(engine::object::GameObject* game_object_ptr);

        /// @brief Safely remove GameObject (set need_remove_marker).
        /// @param game_object_ptr
        virtual void safeRemoveGameObject(engine::object::GameObject* game_object_ptr);

        /// @brief Get the GameObject container in the scene.
        const std::vector<std::unique_ptr<engine::object::GameObject>>& getGameObjects() const { return game_objects_; }

        /// @brief Finds a GameObject by name (returns the first object found).
        /// @param name
        const engine::object::GameObject* findGameObjectByName(std::string_view name) const;

        void setName(std::string_view name) { scene_name_ = name; }
        std::string_view getName() const { return scene_name_; }
        void setInitialized(bool initialized) { is_initialized_ = initialized; }
        bool isInitialized() const { return is_initialized_; }

        engine::core::Context& getContext() const { return context_; }
        engine::scene::SceneManager& getSceneManager() const { return scene_manager_; }

    protected:
        std::string scene_name_;
        engine::core::Context& context_;
        engine::scene::SceneManager& scene_manager_;

        //
        std::unique_ptr<engine::ui::UIManager> ui_manager_;

        bool is_initialized_ = false;
        std::vector<std::unique_ptr<engine::object::GameObject>> game_objects_;
        std::vector<std::unique_ptr<engine::object::GameObject>> pending_additions_;

        void processPendingAdditions();

    };

} // namespace engine::scene

#endif // SCENE_HPP_
