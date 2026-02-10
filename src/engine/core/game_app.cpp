#include "game_app.hpp"
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

}

void GameApp::oneIter() {

}

bool GameApp::init() {
    return true;
}

void GameApp::handleEvents() {

}

void GameApp::update(float delta_time) {

}

void GameApp::render() {

}

void GameApp::close() {

}

bool GameApp::initConfig() {
    return true;
}

bool GameApp::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        spdlog::error("SDL initialization failed! SDL Error: {}", SDL_GetError());
        return false;
    }

    // window_ = SDL_CreateWindow("Title")

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
