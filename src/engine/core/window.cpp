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

} // namespace engine::core
