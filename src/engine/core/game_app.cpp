#include "game_app.hpp"
#include "config.hpp"
#include "../input/input_manager.hpp"
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

    input_manager_->update();

    handleEvents();
    // update(delta_time);
    render();

}

bool GameApp::init() {
    spdlog::trace("Initializing application...");

    if (!initConfig()) return false;
    if (!initSDL()) return false;
    if (!initInputManager()) return false;

    is_running_ = true;
    spdlog::trace("Application initialization successful.");
    return true;
}

void GameApp::handleEvents() {
    if (input_manager_->shouldQuit()) {
        is_running_ = false;
        return;
    }
}

void GameApp::update(float delta_time) {

}

void GameApp::render() {

}

void GameApp::close() {
    if (sdl_renderer_ != nullptr) {
        SDL_DestroyRenderer(sdl_renderer_);
        sdl_renderer_ = nullptr;
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

    spdlog::trace("Configuration initialization successful.");
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

    return true;
}

bool GameApp::initTime() {
    return true;
}

bool GameApp::initResourceManager() {
    return true;
}

bool GameApp::initAudioPlayer() {
    return true;
}

bool GameApp::initRenderer() {
    return true;
}

bool GameApp::initTextRenderer() {
    return true;
}

bool GameApp::initCamera() {
    return true;
}

bool GameApp::initInputManager() {
    try {
        input_manager_ = std::make_unique<engine::input::InputManager>(config_.get());
    }
    catch (const std::exception& exc) {
        spdlog::error("InputManager initialization failed: {}", exc.what());
        return false;
    }
    return true;
}

bool GameApp::initPhysicsEngine() {
    return true;
}

bool GameApp::initGameState() {
    return true;
}

bool GameApp::initContext() {
    return true;
}

bool GameApp::initSceneManager() {
    return true;
}
} // namespace engine::core
