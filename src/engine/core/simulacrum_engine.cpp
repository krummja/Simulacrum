#include "simulacrum_engine.hpp"
// #include "state_manager.hpp"
#include "settings_manager.hpp"

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"

#include "renderlib/gpu_device.hpp"
#include "renderlib/gpu_renderer.hpp"
#include "utilitylib/resource_path.hpp"

#include <cstdlib>
#include <format>
#include <future>
#include <string>
#include <string_view>
#include <vector>
#include <spdlog/spdlog.h>

namespace Simulacrum {

    bool SimulacrumEngine::init(std::string_view title) {
        spdlog::info("Initializing SDL video and gamepad");

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            spdlog::critical("SDL initialization failed: {}", SDL_GetError());
            return false;
        }

        spdlog::info("SDL video online");

        // Initialize resource path resolver
        Simulacrum::ResourcePath::init();

        constexpr int DEFAULT_WIDTH = 1280;
        constexpr int DEFAULT_HEIGHT = 720;

        const std::string settings_path = Simulacrum::ResourcePath::resolve("res/settings.json");

        auto& settings_manager = Simulacrum::SettingsManager::Instance();
        settings_manager.loadFromFile(settings_path);

        return false;
    }

    void SimulacrumEngine::handleEvents() {

    }

    void SimulacrumEngine::setRunning(bool running) { running_ = running; }

    float SimulacrumEngine::getCurrentFPS() const {
        return 0.0f;
    }

    void SimulacrumEngine::update(float delta_time) {

    }

    void SimulacrumEngine::render() {

    }

    void SimulacrumEngine::present() {

    }

    void SimulacrumEngine::processBackgroundTasks() {

    }

    void SimulacrumEngine::setLogicalPresentationMode(SDL_RendererLogicalPresentation mode) {

    }

    bool SimulacrumEngine::isVSyncEnabled() const noexcept {
        return false;
    }

    SDL_RendererLogicalPresentation SimulacrumEngine::getLogicalPresentationMode() const noexcept {
        return logical_presentation_mode_;
    }

    void SimulacrumEngine::clean() {

    }

    bool SimulacrumEngine::setVSyncEnabled(bool enable) {
        return false;
    }

    void SimulacrumEngine::toggleFullscreen() {

    }

    void SimulacrumEngine::setFullscreen(bool enabled) {

    }

    void SimulacrumEngine::setGlobalPause(bool paused) {

    }

    bool SimulacrumEngine::isGlobalPaused() const { return globally_paused_; }

    bool SimulacrumEngine::verifyVsyncState(bool requested) {
        return false;
    }

    void SimulacrumEngine::onWindowResize(const SDL_Event& event) {

    }

    void SimulacrumEngine::onWindowEvent(const SDL_Event& event) {

    }

    void SimulacrumEngine::onDisplayChange(const SDL_Event& event) {

    }

} // namespace Simulacrum
