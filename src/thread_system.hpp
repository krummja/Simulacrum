#pragma once

#include "macros.hpp"

namespace Simulacrum {

    class Simulacrum_API ThreadSystem {
    public:
        static ThreadSystem& Instance() {
            static ThreadSystem instance;
            return instance;
        }

    private:
        ThreadSystem(const ThreadSystem&) = delete;
        ThreadSystem& operator=(const ThreadSystem&) = delete;

        ThreadSystem() = default;
    };

} // namespace Simulacrum
