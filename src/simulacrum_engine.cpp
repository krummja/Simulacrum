#include "simulacrum_engine.hpp"
#include "settings_manager.hpp"
#include "gpu_device.hpp"
#include "gpu_renderer.hpp"
#include "resource_path.hpp"
#include "thread_system.hpp"

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"

#include <cstdlib>
#include <format>
#include <future>
#include <string>
#include <string_view>
#include <vector>
#include <spdlog/spdlog.h>

namespace Simulacrum {

    #define SIMULACRUM_DARK 31, 31, 31, 255

    bool SimulacrumEngine::init(std::string_view title) {
        spdlog::info("Initializing SDL video and gamepad");

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            spdlog::error("SDL initialization failed: {}", SDL_GetError());
            return false;
        }

        spdlog::info("SDL video online");

        // Initialize resource path resolver
        Simulacrum::ResourcePath::init();

        constexpr int DEFAULT_WIDTH = 1280;
        constexpr int DEFAULT_HEIGHT = 720;

        spdlog::info("Loading settings");
        const std::string settings_path = Simulacrum::ResourcePath::resolve("res/settings.json");
        auto& settings_manager = Simulacrum::SettingsManager::Instance();
        settings_manager.loadFromFile(settings_path);

        auto& graphics_settings = settings_manager.getGraphicsSettings();

        const int width = graphics_settings.resolution_width;
        const int height = graphics_settings.resolution_height;
        bool fullscreen = graphics_settings.fullscreen;

        SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "3");
        SDL_SetHint("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", "0");
        SDL_SetHint("SDL_MOUSE_AUTO_CAPTURE", "0");
        SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "0");
        SDL_SetHint("SDL_RENDER_BATCHING", "1");

        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        SDL_SetHint(SDL_HINT_VIDEO_DOUBLE_BUFFER, "1");
        SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");

        if (width <= 0 || height <= 0) {
            window_width_ = 1280;
            window_height_ = 720;
        } else {
            window_width_ = width;
            window_height_ = height;
        }

        windowed_width_ = window_width_;
        windowed_height_ = window_height_;

        SDL_WindowFlags flags = 0;

        if (fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        is_fullscreen_ = fullscreen;

        window_.reset(
            SDL_CreateWindow(
                title.data(),
                window_width_,
                window_height_,
                flags
            )
        );

        if (!window_) {
            spdlog::error("Failed to create window: {}", SDL_GetError());
            return false;
        }

        spdlog::info("Window creation system online");

        // Initialize GPU device and renderer
        auto& gpu_device = Simulacrum::GPUDevice::Instance();

        if (gpu_device.init(window_.get())) {
            auto& gpu_renderer = Simulacrum::GPURenderer::Instance();
            if (gpu_renderer.init()) {
                spdlog::info("SDL3_GPU rendering initialized successfully");
            } else {
                spdlog::warn("GPURenderer init failed - falling back to SDL_Renderer");
                gpu_device.shutdown();
            }
        } else {
            spdlog::warn("GPUDevice init failed - falling back to SDL_Renderer");
        }

        // Set window icon
        // TODO

        // Cache window sizes once for subsequent initialization steps
        int pixel_width = window_width_;
        int pixel_height = window_height_;
        int logical_width = window_width_;
        int logical_height = window_height_;

        if (!SDL_GetWindowSizeInPixels(window_.get(), &pixel_width, &pixel_height)) {
            spdlog::error("Failed to get window pixel size: {}", SDL_GetError());
        }

        if (!SDL_GetWindowSizeInPixels(window_.get(), &logical_width, &logical_height)) {
            spdlog::error("Failed to get window logical size: {}", SDL_GetError());
        }

        renderer_.reset(SDL_CreateRenderer(window_.get(), NULL));

        if (!renderer_) {
            spdlog::error("Failed to create renderer: {}", SDL_GetError());
            return false;
        }

        spdlog::info("GPU rendering system online");

        // Unified VSync initialization with automatic fallback
        auto& settings = Simulacrum::SettingsManager::Instance();
        bool vsync_requested = settings.getGraphicsSettings().vsync;
        vsync_requested_ = vsync_requested;

        spdlog::info(
            "VSync setting from SettingsManager: {}",
            vsync_requested ? "enabled" : "disabled"
        );

        // Create TimestepManager (uses default 60 FPS target and 1/60s fixed timestep)
        timestep_manager_ = std::make_unique<Simulacrum::TimestepManager>();

        bool vsync_set_successfully = SDL_SetRenderVSync(
            renderer_.get(),
            vsync_requested ? 1 : 0
        );

        if (!vsync_set_successfully) {
            spdlog::warn(
                "Failed to {} VSync: {}",
                vsync_requested ? "enable" : "disable",
                SDL_GetError()
            );
        }

        if (vsync_set_successfully) {
            verifyVsyncState(vsync_requested);
        } else {
            timestep_manager_->setSoftwareFrameLimiting(true);
        }

        if (timestep_manager_->isUsingSoftwareFrameLimiting()) {
            spdlog::info(
                "Created: {:.0f} Hz updates, {:.0f} FPS target, software frame limiting",
                timestep_manager_->getUpdateFrequencyHz(),
                timestep_manager_->getTargetFPS()
            );
        } else {
            spdlog::info(
                "Created: {:.0f} Hz updates, VSync enabled",
                timestep_manager_->getUpdateFrequencyHz()
            );
        }

        // Store actual dimensions for UI positioning
        int const actual_width = pixel_width;
        int const actual_height = pixel_height;
        logical_width = actual_width;
        logical_height = actual_height;

        if (!SDL_SetRenderDrawColor(renderer_.get(), SIMULACRUM_DARK)) {
            spdlog::error("Failed to set initial render draw color: {}", SDL_GetError());
        }

        // Use native resolution rendering for crisp, sharp text.
        // Disable logical presentation to render at native resolution
        SDL_RendererLogicalPresentation const presentation_mode = SDL_LOGICAL_PRESENTATION_DISABLED;

        if (!SDL_SetRenderLogicalPresentation(
            renderer_.get(),
            actual_width,
            actual_height,
            presentation_mode
        )) {
            spdlog::error("Failed to set render logical presentation: {}", SDL_GetError());
        }

        spdlog::info("Using native resolution: {}x{}", actual_width, actual_height);

        // Check if icon loaded successfully
        // TODO

        // INITIALIZING GAME RESOURCE LOADING AND MANAGEMENT

        // Calculate DPI-aware font sizes before threading
        dpi_scale_ = 1.0f;

        spdlog::info(
            "DPI scale: {}, window: {}x{}",
            dpi_scale_, window_width_, window_height_
        );

        // Use multiple threads for initialization
        std::vector<std::future<bool>> init_tasks;  // initialization tasks vector
        init_tasks.reserve(12);  // reserve capacity for typical number of init tasks

        init_tasks.push_back(
            Simulacrum::ThreadSystem::Instance().enqueueTaskWithResult(
                []() -> bool {
                    spdlog::info("Creating Event Manager");
                    return true;
                }
            )
        );

        bool all_tasks_succeeded = true;

        for (auto& task : init_tasks) {
            try {
                all_tasks_succeeded &= task.get();
            } catch (const std::exception& exc) {
                all_tasks_succeeded = false;
            }
        }

        if (!all_tasks_succeeded) {
            return false;
        }

        return true;
    }

    void SimulacrumEngine::handleEvents() {
        // TODO
    }

    void SimulacrumEngine::setRunning(bool running) { running_ = running; }

    float SimulacrumEngine::getCurrentFPS() const {
        return 0.0f;
    }

    void SimulacrumEngine::update(float delta_time) {
        // TODO
    }

    void SimulacrumEngine::render() {
        // TODO
    }

    void SimulacrumEngine::present() {
        // TODO
    }

    void SimulacrumEngine::processBackgroundTasks() {
        // TODO
    }

    void SimulacrumEngine::setLogicalPresentationMode(SDL_RendererLogicalPresentation mode) {
        // TODO
    }

    bool SimulacrumEngine::isVSyncEnabled() const noexcept {
        return false;
    }

    SDL_RendererLogicalPresentation SimulacrumEngine::getLogicalPresentationMode() const noexcept {
        return logical_presentation_mode_;
    }

    void SimulacrumEngine::clean() {
        // TODO Double check cleanup after init has been finished

        spdlog::info("Starting shutdown sequence...");

        auto window_to_destroy = std::move(window_);
        auto renderer_to_destroy = std::move(renderer_);

        spdlog::info("Shutting down GPU renderer...");
        Simulacrum::GPURenderer::Instance().shutdown();

        spdlog::info("Shutting down GPU device...");
        Simulacrum::GPUDevice::Instance().shutdown();

        spdlog::info("Destroying renderer...");
        renderer_to_destroy.reset();
        spdlog::info("Renderer destroyed successfully");

        spdlog::info("Destroying window...");
        window_to_destroy.reset();
        spdlog::info("Window destroyed successfully");

        spdlog::info("Calling SDL_Quit...");
        SDL_Quit();

        spdlog::info("Shutdown complete!");
    }

    bool SimulacrumEngine::setVSyncEnabled(bool enable) {
        return false;
    }

    void SimulacrumEngine::toggleFullscreen() {
        // TODO
    }

    void SimulacrumEngine::setFullscreen(bool enabled) {
        // TODO
    }

    void SimulacrumEngine::setGlobalPause(bool paused) {
        // TODO
    }

    bool SimulacrumEngine::isGlobalPaused() const { return globally_paused_; }

    bool SimulacrumEngine::verifyVsyncState(bool requested) {
        return false;
    }

    void SimulacrumEngine::onWindowResize(const SDL_Event& event) {
        // TODO
    }

    void SimulacrumEngine::onWindowEvent(const SDL_Event& event) {
        // TODO
    }

    void SimulacrumEngine::onDisplayChange(const SDL_Event& event) {
        // TODO
    }

} // namespace Simulacrum
