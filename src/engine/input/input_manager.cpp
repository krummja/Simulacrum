#include "input_manager.hpp"
#include "../core/config.hpp"
#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>

namespace engine::input {

    InputManager::InputManager(
        SDL_Renderer* sdl_renderer,
        const engine::core::Config* config
    )
        : sdl_renderer_(sdl_renderer)
    {
        if (!sdl_renderer_) {
            spdlog::error("InputManager: SDL_Renderer is null pointer.");
            throw std::runtime_error("InputManager: SDL_Renderer is null pointer.");
        }
        initializeMappings(config);

        float x, y;
        SDL_GetMouseState(&x, &y);
        mouse_position_ = {x, y};
    }

    void InputManager::update() {
        for (auto& [action_name, state] : action_states_) {
            if (state == ActionState::PRESSED_THIS_FRAME) {
                state = ActionState::HELD_DOWN;
            } else if (state == ActionState::RELEASED_THIS_FRAME) {
                state = ActionState::INACTIVE;
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            processEvent(event);
        }
    }

    void InputManager::processEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                SDL_Scancode scancode = event.key.scancode;
                bool is_down = event.key.down;
                bool is_repeat = event.key.repeat;

                auto it = input_to_actions_map_.find(scancode);
                if (it != input_to_actions_map_.end()) {
                    const std::vector<std::string>& associated_actions = it->second;
                    for (const auto& action_name : associated_actions) {
                        updateActionState(action_name, is_down, is_repeat);
                    }
                }

                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                Uint32 button = event.button.button;
                bool is_down = event.button.down;

                auto it = input_to_actions_map_.find(button);
                if (it != input_to_actions_map_.end()) {
                    const std::vector<std::string>& associated_actions = it->second;
                    for (const auto& action_name : associated_actions) {
                        updateActionState(action_name, is_down, false);
                    }
                }

                mouse_position_ = {event.button.x, event.button.y};
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                mouse_position_ = {event.motion.x, event.motion.y};
                break;
            }

            case SDL_EVENT_QUIT:
                should_quit_ = true;
                break;

                default:
                break;
        }
    }

    bool InputManager::isActionDown(std::string_view action_name) const {
        if (auto it = action_states_.find(std::string(action_name)); it != action_states_.end()) {
            return it->second == ActionState::PRESSED_THIS_FRAME || it->second == ActionState::HELD_DOWN;
        }
        return false;
    }

    bool InputManager::isActionPressed(std::string_view action_name) const {
        if (auto it = action_states_.find(std::string(action_name)); it != action_states_.end()) {
            return it->second == ActionState::PRESSED_THIS_FRAME;
        }
        return false;
    }

    bool InputManager::isActionReleased(std::string_view action_name) const {
        if (auto it = action_states_.find(std::string(action_name)); it != action_states_.end()) {
            return it->second == ActionState::RELEASED_THIS_FRAME;
        }
        return false;
    }

    bool InputManager::shouldQuit() const {
        return should_quit_;
    }

    void InputManager::setShouldQuit(bool should_quit) {
        should_quit_ = should_quit;
    }

    glm::vec2 InputManager::getMousePosition() const {
        return mouse_position_;
    }

    glm::vec2 InputManager::getLogicalMousePosition() const {
        glm::vec2 logical_pos;
        SDL_RenderCoordinatesFromWindow(
            sdl_renderer_,
            mouse_position_.x,
            mouse_position_.y,
            &logical_pos.x,
            &logical_pos.y
        );
        return logical_pos;
    }

    void InputManager::initializeMappings(const engine::core::Config* config) {
        actions_to_keyname_map_ = config ->input_mappings_;
        input_to_actions_map_.clear();
        action_states_.clear();

        if (actions_to_keyname_map_.find("MouseLeftClick") == actions_to_keyname_map_.end()) {
            spdlog::debug("The 'MouseLeftClick' action is not defined in config. Adding a default mapping to 'MouseLeft'");
            actions_to_keyname_map_["MouseLeftClick"] = {"MouseLeft"};
        }

        if (actions_to_keyname_map_.find("MouseRightClick") == actions_to_keyname_map_.end()) {
            spdlog::debug("The 'MouseRightClick' action is not defined in config. Adding a default mapping to 'MouseRight'");
            actions_to_keyname_map_["MouseRightClick"] = {"MouseRight"};
        }

        for (const auto& [action_name, key_names] : actions_to_keyname_map_) {
            action_states_[action_name] = ActionState::INACTIVE;
            spdlog::trace("Mapping actions: {}", action_name);

            for (const auto& key_name : key_names) {
                SDL_Scancode scancode = scancodeFromString(key_name);
                Uint32 mouse_button = mouseButtonFromString(key_name);

                if (scancode != SDL_SCANCODE_UNKNOWN) {
                    input_to_actions_map_[scancode].push_back(action_name);
                    spdlog::trace("Mapping keys: {} (scancode: {} to action: {})", key_name, static_cast<int>(scancode), action_name);
                } else if (mouse_button != 0) {
                    input_to_actions_map_[mouse_button].push_back(action_name);
                    spdlog::trace("Mapping mouse buttons: {} (button ID: {} to action: {})", key_name, static_cast<int>(mouse_button), action_name);
                } else {
                    spdlog::warn("Input mapping warning: Unknown key or button name '{}' for action '{}'", key_name, action_name);
                }
            }
        }
    }

    SDL_Scancode InputManager::scancodeFromString(std::string_view key_name) {
        return SDL_GetScancodeFromName(key_name.data());
    }

    Uint32 InputManager::mouseButtonFromString(std::string_view button_name) {
        if (button_name == "MouseLeft") return SDL_BUTTON_LEFT;
        if (button_name == "MouseMiddle") return SDL_BUTTON_MIDDLE;
        if (button_name == "MouseRight") return SDL_BUTTON_RIGHT;
        if (button_name == "MouseX1") return SDL_BUTTON_X1;
        if (button_name == "MouseX2") return SDL_BUTTON_X2;
        return 0;
    }

    void InputManager::updateActionState(std::string_view action_name, bool is_input_active, bool is_repeat_event) {
        auto it = action_states_.find(std::string(action_name));
        if (it == action_states_.end()) {
            return;
        }

        if (is_input_active) {
            if (is_repeat_event) {
                it->second = ActionState::HELD_DOWN;
            } else {
                it->second = ActionState::PRESSED_THIS_FRAME;
            }
        } else {
            it->second = ActionState::RELEASED_THIS_FRAME;
        }
    }

} // namespace engine::input
