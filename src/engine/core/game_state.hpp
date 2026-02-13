#ifndef GAME_STATE_HPP_
#define GAME_STATE_HPP_

#include <optional>
#include <string>
#include <iostream>
#include <type_traits>
#include <glm/vec2.hpp>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_render.h>

namespace engine::core {

    enum class State {
        Title,
        Playing,
        Paused,
        GameOver,
        // Add additional as needed, e.g. SettingsMenum etc.
    };

    class GameState final {
    public:
        /// @brief Constructor; initializes the game.
        /// @param window SDL window
        /// @param renderer SDL renderer
        /// @param initial_state Initial state of the game. Default is Title.
        explicit GameState(SDL_Window* window, SDL_Renderer* renderer, State initial_state = State::Title);

        State getCurrentState() const { return current_state_; }
        void setState(State new_state);
        glm::vec2 getWindowSize() const;
        void setWindowSize(const glm::vec2& window_size);
        glm::vec2 getLogicalSize() const;
        void setLogicalSize(const glm::vec2& logical_size);

        bool isState(State state) const {
            return current_state_ == state;
        }

        bool isInTitle() const {
            return current_state_ == State::Title;
        }

        bool isInPlaying() const {
            return current_state_ == State::Playing;
        }

        bool isInPaused() const {
            return current_state_ == State::Paused;
        }

        bool isInGameOver() const {
            return current_state_ == State::GameOver;
        }

    private:
        /// @brief SDL window, used to get the window size
        SDL_Window* window_ = nullptr;

        /// @brief SDL renderer for getting logical resolution
        SDL_Renderer* renderer_ = nullptr;

        /// @brief Current game state
        State current_state_ = State::Title;
    };

} // namespace engine::core


#endif // GAME_STATE_HPP_
