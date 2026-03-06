#include "input_manager.hpp"
#include "simulacrum_engine.hpp"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <format>
#include <memory>

namespace Simulacrum
{

  InputManager::InputManager()
    : key_states_(nullptr)
    , mouse_position_(std::make_unique<Vector2D>(0, 0))
  {
    button_states_.reserve(1);
    mouse_button_states_.reserve(3);

    for (int i = 0; i < 3; i++)
    {
      mouse_button_states_.push_back(false);
    }
  }

  bool InputManager::init()
  {
    if (is_initialized_)
    {
      spdlog::warn("InputManager already initialized");
      return true;
    }

    spdlog::info("Initializing InputManager");

    key_states_ = SDL_GetKeyboardState(nullptr);
    if (!key_states_)
    {
      spdlog::warn("No keyboard state available - device may not have keyboard input");
    }

    if (!mouse_position_)
    {
      mouse_position_ = std::make_unique<Vector2D>(0, 0);
    }

    is_initialized_ = true;
    spdlog::info("InputManager initialized successfully");
    return true;
  }

  void InputManager::reset()
  {
    mouse_button_states_[LEFT] = false;
    mouse_button_states_[MIDDLE] = false;
    mouse_button_states_[RIGHT] = false;
  }

  bool InputManager::isKeyJustPressed(SDL_Scancode key) const
  {
    return std::any_of(
      pressed_this_frame_.begin(),
      pressed_this_frame_.end(),
      [key](SDL_Scancode pressed_key)
      {
        return pressed_key == key;
      }
    );
  }

  bool InputManager::isKeyPressed(SDL_Scancode key) const
  {
    if (key_states_ != nullptr)
    {
      return key_states_[key] == 1;
    }
    return false;
  }

  bool InputManager::getMouseButtonState(int button_number) const
  {
    if (button_number < 0 || button_number >= static_cast<int>(mouse_button_states_.size()))
    {
      return false;
    }

    return mouse_button_states_[button_number];
  }

  const Vector2D& InputManager::getMousePosition() const
  {
    return *mouse_position_;
  }

  void InputManager::clearFrameInput()
  {
    pressed_this_frame_.clear();
  }

  void InputManager::update()
  {
    clearFrameInput();
  }

  void InputManager::onKeyDown(const SDL_Event& event)
  {
    key_states_ = SDL_GetKeyboardState(0);

    spdlog::debug("Key: {}", SDL_GetKeyName(event.key.key));

    bool already_tracked = std::any_of(
      pressed_this_frame_.begin(),
      pressed_this_frame_.end(),
      [scancode = event.key.scancode](SDL_Scancode pressed_key)
      {
        return pressed_key == scancode;
      }
    );

    if (!already_tracked)
    {
      pressed_this_frame_.push_back(event.key.scancode);
    }
  }

  void InputManager::onKeyUp(const SDL_Event& event) {}

  void InputManager::onMouseMove(const SDL_Event& event)
  {
    // GPU renders at pixel resolution, but SDL mouse events are in window coordinates.
    // Scale by pixel density to convert window coords to pixel coords.
    float scale = 1.0f;

    SDL_Window* window = SimulacrumEngine::Instance().getWindow();
    if (window)
    {
      scale = SDL_GetWindowPixelDensity(window);
    }

    mouse_position_->x = event.motion.x * scale;
    mouse_position_->y = event.motion.y * scale;
  }

  void InputManager::onMouseButtonDown(const SDL_Event& event)
  {
    if (event.button.button == SDL_BUTTON_LEFT)
    {
      mouse_button_states_[LEFT] = true;
    }

    if (event.button.button == SDL_BUTTON_MIDDLE)
    {
      mouse_button_states_[MIDDLE] = true;
    }

    if (event.button.button == SDL_BUTTON_RIGHT)
    {
      mouse_button_states_[RIGHT] = true;
    }
  }

  void InputManager::onMouseButtonUp(const SDL_Event& event)
  {
    if (event.button.button == SDL_BUTTON_LEFT)
    {
      mouse_button_states_[LEFT] = false;
    }

    if (event.button.button == SDL_BUTTON_MIDDLE)
    {
      mouse_button_states_[MIDDLE] = false;
    }

    if (event.button.button == SDL_BUTTON_RIGHT)
    {
      mouse_button_states_[RIGHT] = false;
    }
  }

  void InputManager::clean()
  {
    if (is_shutdown_)
    {
      return;
    }

    mouse_button_states_.clear();
    is_shutdown_ = true;
  }

} // namespace Simulacrum
