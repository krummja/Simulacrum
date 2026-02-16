#include "game_app.hpp"
#include "time.hpp"
#include "context.hpp"
#include "config.hpp"
#include "game_state.hpp"
#include "../resource/resource_manager.hpp"
#include "../render/renderer.hpp"
#include "../render/text_renderer.hpp"
#include "../render/gpu_renderer.hpp"
#include "../render/camera.hpp"
#include "../input/input_manager.hpp"
#include "../scene/scene_manager.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::core {

    GameApp::GameApp() = default;

    GameApp::~GameApp() {
        if (is_running_) {
            spdlog::warn("GameApp was destroyed without explicit shutdown. Closing...");
            close();
        }
    }

    void GameApp::run() {

        if (!init()) {
            spdlog::error("Application failed to initialize and cannot run.");
            return;
        }

        while (is_running_) {
            oneIter();
        }

        close();
    }

    void GameApp::oneIter() {
        if (!is_running_) return;

        time_->update();
        float delta_time = time_->getDeltaTime();
        input_manager_->update();

        handleEvents();
        update(delta_time);
        render();
    }

    void GameApp::registerSceneSetup(std::function<void(engine::scene::SceneManager&)> func) {
        scene_setup_func_ = std::move(func);
    }

    bool GameApp::init() {
        spdlog::trace("Starting up application...");

        if (!scene_setup_func_) {
            return false;
        }

        spdlog::trace("Initializing engine modules...");

        if (!initConfig()) return false;
        if (!initSDL()) return false;
        if (!initTime()) return false;
        if (!initResourceManager()) return false;
        if (!initAudioPlayer()) return false;
        if (!initRenderer()) return false;
        if (!initGPURenderer()) return false;
        if (!initCamera()) return false;
        if (!initTextRenderer()) return false;
        if (!initInputManager()) return false;
        if (!initPhysicsEngine()) return false;
        if (!initGameState()) return false;

        spdlog::trace("All modules started successfully!");

        if (!initContext()) return false;
        if (!initSceneManager()) return false;

        scene_setup_func_(*scene_manager_);

        is_running_ = true;
        spdlog::trace("Application startup complete!");
        return true;
    }

    void GameApp::handleEvents() {
        if (input_manager_->shouldQuit()) {
            spdlog::trace("GameApp has received an exit request from InputManager.");
            is_running_ = false;
            return;
        }

        scene_manager_->handleInput();
    }

    void GameApp::update(float delta_time) {
        scene_manager_->update(delta_time);
    }

    void GameApp::render() {

        // 1. Clear screen
        renderer_->clearScreen();

        // 2. Render active scene
        scene_manager_->render();

        // 3. Update screen display
        renderer_->present();
    }

    void GameApp::close() {
        if (sdl_renderer_ != nullptr) {
            SDL_DestroyRenderer(sdl_renderer_);
            sdl_renderer_ = nullptr;
        }

        if (gpu_device_ != nullptr) {
            SDL_DestroyGPUDevice(gpu_device_);
            gpu_device_ = nullptr;
        }

        if (window_ != nullptr) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }

        SDL_Quit();
        is_running_ = false;
    }

    bool GameApp::initConfig() {
        try {
            config_ = std::make_unique<engine::core::Config>("assets/config.json");
        }

        catch (const std::exception& exc) {
            spdlog::error("Configuration initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  Configuration initialization successful.");
        return true;
    }

    bool GameApp::initSDL() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            spdlog::error("SDL initialization failed! SDL Error: {}", SDL_GetError());
            return false;
        }

        window_ = SDL_CreateWindow(
            config_->window_title_.c_str(),
            config_->window_width_,
            config_->window_height_,
            SDL_WINDOW_RESIZABLE
        );

        if (window_ == nullptr) {
            spdlog::error("Unable to create SDL window! SDL Error: {}", SDL_GetError());
            return false;
        }

        sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (sdl_renderer_ == nullptr) {
            spdlog::error("Unable to create SDL renderer! SDL Error: {}", SDL_GetError());
            return false;
        }

        gpu_device_ = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
        if (gpu_device_ == nullptr) {
            spdlog::error("No GPU Device available to bind to window.");
            return false;
        }

        SDL_ClaimWindowForGPUDevice(gpu_device_, window_);

        // Set renderer to support transparent colors
        SDL_SetRenderDrawBlendMode(sdl_renderer_, SDL_BLENDMODE_BLEND);

        // Setting VSync (Note: When VSync is on, the driver will try to limit the frame
        // rate to the monitor refresh rate, which may overwrite the target_fps we set
        // manually)
        int vsync_mode = config_->vsync_enabled_ ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
        SDL_SetRenderVSync(sdl_renderer_, vsync_mode);

        // Set the logical resolution to half the window size (for pixel games)
        SDL_SetRenderLogicalPresentation(
            sdl_renderer_,
            config_->window_width_,
            config_->window_height_,
            SDL_LOGICAL_PRESENTATION_LETTERBOX
        );

        spdlog::trace("  SDL initialization successful.");
        return true;
    }

    bool GameApp::initGPURenderer() {
        try {
            gpu_renderer_ = std::make_unique<engine::render::GPURenderer>(gpu_device_);
        }

        catch (const std::exception& exc) {
            spdlog::error("GPURenderer initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  GPURenderer initialization successful.");
        return true;
    }

    bool GameApp::initTime() {
        try {
            time_ = std::make_unique<Time>();
        }

        catch (const std::exception& exc) {
            spdlog::error("Time management initialization failed: {}", exc.what());
            return false;
        }

        time_->setTargetFps(config_->target_fps_);
        spdlog::trace("  Time initialization successful.");
        return true;
    }

    bool GameApp::initResourceManager() {
        try {
            resource_manager_ = std::make_unique<engine::resource::ResourceManager>(sdl_renderer_);
        }

        catch (const std::exception& exc) {
            spdlog::error("Resource management initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  ResourceManager initialization successful.");
        return true;
    }

    bool GameApp::initAudioPlayer() {
        // spdlog::trace("  AudioPlayer initialization successful.");
        spdlog::warn("AudioPlayer not yet implemented!");
        return true;
    }

    bool GameApp::initRenderer() {
        try {
            renderer_ = std::make_unique<engine::render::Renderer>(sdl_renderer_, resource_manager_.get());
        }

        catch (const std::exception& exc) {
            spdlog::error("Renderer initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  Renderer initialization successful.");
        return true;
    }

    bool GameApp::initTextRenderer() {
        try {
            text_renderer_ = std::make_unique<engine::render::TextRenderer>(sdl_renderer_, resource_manager_.get());
        }

        catch (const std::exception& exc) {
            spdlog::error("Text renderer initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  TextRenderer initialization successful.");
        return true;
    }

    bool GameApp::initCamera() {
        try {
            camera_ = std::make_unique<engine::render::Camera>(
                glm::vec2(config_->window_width_ / 2, config_->window_height_ / 2)
            );
        }

        catch (const std::exception& exc) {
            spdlog::error("Camera initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  Camera initialization successful.");
        return true;
    }

    bool GameApp::initInputManager() {
        try {
            input_manager_ = std::make_unique<engine::input::InputManager>(
                sdl_renderer_,
                config_.get()
            );
        }

        catch (const std::exception& exc) {
            spdlog::error("InputManager initialization failed: {}", exc.what());
            return false;
        }
        return true;
    }

    bool GameApp::initPhysicsEngine() {
        // spdlog::trace("  PhysicsEngine initialization successful.");
        spdlog::warn("PhysicsEngine not yet implemented!");
        return true;
    }

    bool GameApp::initGameState() {
        try {
            game_state_ = std::make_unique<engine::core::GameState>(window_, sdl_renderer_);
        }

        catch (const std::exception& exc) {
            spdlog::error("GameState initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("  GameState initialization successful.");
        return true;
    }

    bool GameApp::initContext() {
        spdlog::trace("Initializing Context...");

        try {
            context_ = std::make_unique<engine::core::Context>(
                *input_manager_,
                *renderer_,
                *text_renderer_,
                *resource_manager_,
                *game_state_
            );
        }

        catch (const std::exception& exc) {
            spdlog::error("Context initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("Context initialization successful.");
        return true;
    }

    bool GameApp::initSceneManager() {
        spdlog::trace("Initializing SceneManager...");

        try {
            scene_manager_ = std::make_unique<engine::scene::SceneManager>(*context_);
        }

        catch (const std::exception& exc) {
            spdlog::error("Scene Manager initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("Scene Manager initialization successful.");
        return true;
    }

} // namespace engine::core
