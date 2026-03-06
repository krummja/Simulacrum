#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "entity_state.hpp"

namespace Simulacrum {

    class EntityStateManager {
    public:
        EntityStateManager();
        void addState(const std::string& state_name, std::unique_ptr<EntityState> state);
        void setState(const std::string& state_name);
        std::string getCurrentStateName() const;
        bool hasState(const std::string& state_name) const;

        void update(float delta_time);
        ~EntityStateManager();

    private:
        std::unordered_map<std::string, std::shared_ptr<EntityState>> states_;
        std::weak_ptr<EntityState> current_state_;
    };

} // namespace Simulacrum

