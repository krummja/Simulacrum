#pragma once

#include <SDL3/SDL.h>
#include <utility>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Simulacrum {

    using Vector2D = glm::vec2;

    enum mouse_buttons {
        LEFT = 0,
        MIDDLE = 1,
        RIGHT = 2
    };

    class InputManager {
    public:
        ~InputManager() {
            if (!is_shutdown_) {
                clean();
            }
        }

        static InputManager& Instance() {
            static InputManager instance;
            return instance;
        }

        bool init();

        void update();

        void reset();

        void clean();

        bool isInitialized() const { return is_initialized_; }

        bool isShutdown() const { return is_shutdown_; }

        bool isKeyPressed(SDL_Scancode key) const;
        bool isKeyJustPressed(SDL_Scancode key) const;
        void clearFrameInput();

        bool getMouseButtonState(int button_number) const;
        const Vector2D& getMousePosition() const;

        void onKeyDown(const SDL_Event& event);
        void onKeyUp(const SDL_Event& event);
        void onMouseMove(const SDL_Event& event);
        void onMouseButtonDown(const SDL_Event& event);
        void onMouseButtonUp(const SDL_Event& event);

    private:
        const bool* key_states_{nullptr};
        std::vector<SDL_Scancode> pressed_this_frame_{};

        std::vector<bool> button_states_{};
        std::vector<bool> mouse_button_states_{};
        std::unique_ptr<Vector2D> mouse_position_{nullptr};

        bool is_initialized_{false};
        bool is_shutdown_{false};

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        InputManager();
    };

} // namespace Simulacrum
