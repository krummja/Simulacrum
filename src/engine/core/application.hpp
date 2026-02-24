#pragma once

#include <memory>
#include <functional>

struct SDL_Window;
struct SDL_GPUDevice;

namespace engine::input {
    class InputManager;
}

namespace engine::render {
    class GPURenderer;
}

namespace engine::core {
    class Time;
    class Config;
    class Context;

    class Application final {
    public:
        Application();
        ~Application();

        void run();
        void oneIter();

        // Copy operations
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // Move operations
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

    private:
        SDL_Window* window_ = nullptr;
        SDL_GPUDevice* gpu_device_ = nullptr;

        bool is_running_ = false;

        std::unique_ptr<engine::render::GPURenderer> gpu_renderer_;
        std::unique_ptr<engine::core::Config> config_;
        std::unique_ptr<engine::core::Time> time_;
        std::unique_ptr<engine::input::InputManager> input_manager_;

        void handleEvents();
        void update(float delta_time);
        void render();
        void close();

        [[nodiscard]] bool init();

        [[nodiscard]] bool initConfig();
        [[nodiscard]] bool initSDL();
        [[nodiscard]] bool initGPURenderer();
        [[nodiscard]] bool initTime();
        [[nodiscard]] bool initInputManager();
    };

} // namespace engine::core
