#pragma once

#include "config.hpp"
#include <SDL3/SDL.h>

namespace engine::core {

    class Window {
    public:
        Window(Config* config);
        ~Window();

        SDL_Window* get() { return window_; }

        void render();
        void update();
        SDL_Surface* getSurface() const;

        // Copy operations
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

    private:
        Config* config_ = nullptr;
        SDL_Window* window_ = nullptr;
    };

} // namespace engine::core
