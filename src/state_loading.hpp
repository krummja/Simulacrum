#pragma once

#include "state.hpp"
#include <atomic>
#include <future>
#include <string>

namespace Simulacrum {

    class LoadingState : public State {
    public:
        LoadingState() = default;

        void configure(const std::string& target_state_name);



    private:

    };

} // namespace Simulacrum
