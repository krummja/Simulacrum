#include "EntityStateManager.hpp"
#include "EntityState.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <format>
#include <stdexcept>

namespace Simulacrum
{

  EntityStateManager::EntityStateManager() : current_state_() {}

  EntityStateManager::~EntityStateManager()
  {
    current_state_.reset();
  }

  void EntityStateManager::addState(const std::string& state_name, std::unique_ptr<EntityState> state)
  {
    if (states_.find(state_name) != states_.end())
    {
      spdlog::error("State already exists: {}", state_name);
      throw std::invalid_argument(std::format("State already exists: {}", state_name));
    }

    states_[state_name] = std::shared_ptr<EntityState>(state.release());
  }

  void EntityStateManager::setState(const std::string& state_name)
  {
    auto it = states_.find(state_name);
    if (it != states_.end())
    {
      if (auto current = current_state_.lock())
      {
        current->exit();
      }

      current_state_ = it->second;

      if (auto current = current_state_.lock())
      {
        current->enter();
      }
    }

    else
    {
      if (auto current = current_state_.lock())
      {
        current->exit();
      }

      spdlog::error("State not found: {}", state_name);
      current_state_.reset();
    }
  }

  std::string EntityStateManager::getCurrentStateName() const
  {
    if (auto current = current_state_.lock())
    {
      auto it = std::find_if(states_.begin(), states_.end(),
        [&current](const auto& pair)
        {
          return pair.second == current;
        }
      );

      if (it != states_.end())
      {
        return it->first;
      }
    }

    return "";
  }

  bool EntityStateManager::hasState(const std::string& state_name) const
  {
    return states_.find(state_name) != states_.end();
  }

  void EntityStateManager::update(float delta_time)
  {
    if (auto current = current_state_.lock())
    {
      current->update(delta_time);
    }
  }

} // namespace Simulacrum
