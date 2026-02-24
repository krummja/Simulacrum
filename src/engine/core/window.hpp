#pragma once

#include "config.hpp"
#include <SDL3/SDL.h>

namespace engine::core {

    class Window {
    public:
        Window(Config* config);

        SDL_Window* get() { return window_; }

    private:
        Config* config_ = nullptr;
        SDL_Window* window_ = nullptr;
    };

} // namespace engine::core
