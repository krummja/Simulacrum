#include "application.hpp"
#include "time.hpp"
#include "config.hpp"
#include "../render/gpu_renderer.hpp"
#include "../input/input_manager.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::core {

    Application::Application() = default;

    Application::~Application() {
        if (is_running_) {
            spdlog::warn("Application was destroyed without explicit shutdown. Closing...");
            close();
        }
    }

    bool Application::init() {
        spdlog::trace("Starting application...");

        spdlog::trace("  Initializing core modules...");

        if (!initConfig()) return false;
        if (!initSDL()) return false;
        if (!initTime()) return false;
        if (!initInputManager()) return false;

        spdlog::trace("  All core modules initialized successfully!");

        is_running_ = true;

        spdlog::trace("Application startup complete!");
        return true;
    }

    void Application::run() {
        if (!init()) {
            spdlog::error("Application failed to initialize and cannot run.");
            return;
        }

        while (is_running_) {
            oneIter();
        }

        close();
    }

    void Application::oneIter() {
        if (!is_running_) {
            return;
        }

        time_->update();
        float delta_time = time_->getDeltaTime();
        input_manager_->update();

        handleEvents();
        update(delta_time);
        render();
    }

    void Application::handleEvents() {
        if (input_manager_->shouldQuit()) {
            is_running_ = false;
            return;
        }
    }

    void Application::update(float delta_time) {

    }

    void Application::render() {
        gpu_renderer_->render();
    }

    void Application::close() {
        if (gpu_renderer_ != nullptr) {
            gpu_renderer_->close();
            gpu_renderer_ = nullptr;
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

    bool Application::initConfig() {
        try {
            config_ = std::make_unique<engine::core::Config>("assets/config.json");
        } catch (const std::exception& exc) {
            spdlog::error("Config initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("    Config initialization successful.");
        return true;
    }

    bool Application::initSDL() {
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

        gpu_device_ = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
        if (gpu_device_ == nullptr) {
            spdlog::error("No GPU device available to bind to window.");
            return false;
        }

        spdlog::debug("    Claiming window for GPU device");
        SDL_ClaimWindowForGPUDevice(gpu_device_, window_);

        spdlog::debug("    Initializing GPU renderer...");

        if (!initGPURenderer()) {
            spdlog::error("Failed to initialize GPURenderer");
            return false;
        }

        gpu_renderer_->init();

        spdlog::trace("    SDL initialization successful.");
        return true;
    }

    bool Application::initGPURenderer() {
        try {
            gpu_renderer_ = std::make_unique<engine::render::GPURenderer>(
                gpu_device_,
                window_
            );
        } catch (const std::exception& exc) {
            spdlog::error("GPURenderer initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("    GPURenderer initialization successful.");
        return true;
    }

    bool Application::initTime() {
        try {
            time_ = std::make_unique<Time>();
        } catch (const std::exception& exc) {
            spdlog::error("Time initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("    Time initialization successful.");
        return true;
    }

    bool Application::initInputManager() {
        try {
            input_manager_ = std::make_unique<engine::input::InputManager>(config_.get());
        } catch (const std::exception& exc) {
            spdlog::error("InputManager initialization failed: {}", exc.what());
            return false;
        }

        spdlog::trace("    InputManager initialization successful");
        return true;
    }
}
