#pragma once

#include <atomic>
#include <cstdint>

namespace Simulacrum {

    class UniqueId {
    public:
        using IDType = uint64_t;

        static IDType generate() {
            return next_id++;
        }

        static constexpr IDType INVALID_ID = 0;

    private:
        static inline std::atomic<IDType> next_id{1};
    };

} // namespace Simulacrum
