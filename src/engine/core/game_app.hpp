#ifndef GAME_APP_HPP_
#define GAME_APP_HPP_

#include <memory>
#include <functional>

// Forward declaration to reduce header file dependencies and increase compilation speed
struct SDL_Window;
struct SDL_Renderer;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {
    class Renderer;
    class Camera;
    class TextRenderer;
}

namespace engine::input {
    class InputManager;
}

namespace engine::physics {
    class PhysicsEngine;
}

namespace engine::scene {
    class SceneManager;
}

namespace engine::audio {
    class AudioPlayer;
}

namespace engine::core {
    class Time;
    class Config;
    class Context;
    class GameState;

    /**
     * @brief Main game application class, initialize SDL, manage game loop
     */
    class GameApp final {
    public:
        GameApp();
        ~GameApp();
        void run();
        void oneIter();
        void registerSceneSetup(std::function<void(engine::scene::SceneManager&)> func);

        GameApp(const GameApp&) = delete;
        GameApp& operator=(const GameApp&) = delete;
        GameApp(GameApp&&) = delete;
        GameApp& operator=(GameApp&&) = delete;

    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* sdl_renderer_ = nullptr;
        bool is_running_ = false;

        std::function<void(engine::scene::SceneManager&)> scene_setup_func_;

        std::unique_ptr<engine::core::Time> time_;
        std::unique_ptr<engine::resource::ResourceManager> resource_manager_;
        std::unique_ptr<engine::core::Config> config_;
        std::unique_ptr<engine::input::InputManager> input_manager_;
        std::unique_ptr<engine::core::Context> context_;
        std::unique_ptr<engine::scene::SceneManager> scene_manager_;
        std::unique_ptr<engine::core::GameState> game_state_;

        void handleEvents();
        void update(float delta_time);
        void render();
        void close();

        // The initialization/creation function of each module is called in init()
        [[nodiscard]] bool init();
        [[nodiscard]] bool initConfig();
        [[nodiscard]] bool initSDL();
        [[nodiscard]] bool initTime();
        [[nodiscard]] bool initResourceManager();
        [[nodiscard]] bool initAudioPlayer();
        [[nodiscard]] bool initRenderer();
        [[nodiscard]] bool initTextRenderer();
        [[nodiscard]] bool initCamera();
        [[nodiscard]] bool initInputManager();
        [[nodiscard]] bool initPhysicsEngine();
        [[nodiscard]] bool initGameState();
        [[nodiscard]] bool initContext();
        [[nodiscard]] bool initSceneManager();
    };

} // namespace engine::core

#endif // GAME_APP_HPP_
