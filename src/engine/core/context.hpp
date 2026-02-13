#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

namespace engine::input {
    class InputManager;
}

namespace engine::render {
    class Renderer;
    class Camera;
    class TextRenderer;
}

namespace engine::resource {
    class ResourceManager;
}

namespace engine::physics {
    class PhysicsEngine;
}

namespace engine::audio {
    class AudioPlayer;
}

namespace engine::core {
    class GameState;

    class Context final {
    public:
        Context(engine::input::InputManager& input_manager,
                engine::core::GameState& game_state);

        Context(const Context&) = delete;
        Context operator=(const Context&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;

        engine::input::InputManager& getInputManager() const { return input_manager_; }
        engine::core::GameState& getGameState() const { return game_state_; }

    private:
        engine::input::InputManager& input_manager_;
        engine::core::GameState& game_state_;
    };

} // namespace engine::core

#endif // CONTEXT_HPP_
