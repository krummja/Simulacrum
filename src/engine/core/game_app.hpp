#pragma once
#include <memory>
#include <functional>

// Forward declaration to reduce header file dependencies and increase compilation speed
struct SDL_Window;
struct SDL_Renderer;

namespace engine::render {
class Renderer;
class Camera;
class TextRenderer;
} // namespace engine::render

namespace engine::core {
class Time;
class Config;
class Context;
class GameState;

/**
 * @brief Main game application class, initialize SDL, manage game loop
 */
class GameApp final {

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* sdl_renderer_ = nullptr;
    bool is_running_ = false;

public:
    GameApp();
    ~GameApp();
    void run();
    void oneIter();
    // void registerSceneSetup();

    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;
    GameApp(GameApp&&) = delete;
    GameApp& operator=(GameApp&&) = delete;

private:
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