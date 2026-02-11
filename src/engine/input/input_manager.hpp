#ifndef INPUT_MANAGER_HPP_
#define INPUT_MANAGER_HPP_

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <variant>
#include <SDL3/SDL_render.h>

namespace engine::core {
    class Config;
}

namespace engine::input {

    enum class ActionState {
        INACTIVE,
        PRESSED_THIS_FRAME,
        HELD_DOWN,
        RELEASED_THIS_FRAME,
    };

    class InputManager {
    public:
        InputManager(const engine::core::Config* config);

        void update();
        bool shouldQuit() const;

    private:
        std::unordered_map<std::string, std::vector<std::string>> actions_to_keyname_map_;
        std::unordered_map<std::variant<SDL_Scancode, Uint32>, std::vector<std::string>> input_to_actions_map_;
        std::unordered_map<std::string, ActionState> action_states_;

        bool should_quit_ = false;
        void processEvent(const SDL_Event& event);
        SDL_Scancode scancodeFromString(std::string_view key_name);
    };

} // namespace engine::input

#endif // INPUT_MANAGER_HPP_
