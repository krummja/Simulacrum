#include "input_manager.hpp"
#include "../core/config.hpp"
#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>

namespace engine::input {

    InputManager::InputManager(
        // SDL_Renderer* sdl_renderer,
        const engine::core::Config* config
    )
        // : sdl_renderer_(sdl_renderer)
    {}

    void InputManager::update() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            processEvent(event);
        }
    }

    void InputManager::processEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                should_quit_ = true;
                break;
            default:
                break;
        }
    }

    bool InputManager::shouldQuit() const {
        return should_quit_;
    }

} // namespace engine::input
