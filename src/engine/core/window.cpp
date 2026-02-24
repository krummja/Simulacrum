#include "window.hpp"

namespace engine::core {

    Window::Window(Config* config): config_(config) {
        window_ = SDL_CreateWindow(
            config_->window_title_.c_str(),
            config_->window_width_,
            config_->window_height_,
            SDL_WINDOW_RESIZABLE
        );
    }

    Window::~Window() {
        if (window_ && SDL_WasInit(SDL_INIT_VIDEO)) {
            SDL_DestroyWindow(window_);
        }
    }

    SDL_Surface* Window::getSurface() const {
        return SDL_GetWindowSurface(window_);
    }

    void Window::update() {
        SDL_UpdateWindowSurface(window_);
    }

    void Window::render() {
        const auto* fmt = SDL_GetPixelFormatDetails(
            getSurface()->format
        );

        SDL_FillSurfaceRect(
            getSurface(),
            nullptr,
            SDL_MapRGB(fmt, nullptr, 50, 50, 50)
        );
    }

} // namespace engine::core
