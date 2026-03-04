#pragma once

#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace Simulacrum {

    class ResourceHandle {
    public:
        using HandleId = uint32_t;
        using Generation = uint16_t;

        static constexpr HandleId INVALID_ID = 0;
        static constexpr Generation INVALID_GENERATION = 0;

        /// Default constructor creates invalid handle
        constexpr ResourceHandle() noexcept
            : id_(INVALID_ID)
            , generation_(INVALID_GENERATION)
        {}

        constexpr ResourceHandle(HandleId id, Generation generation) noexcept
            : id_(id)
            , generation_{generation}
        {}

        constexpr HandleId getId() const noexcept { return id_; }
        constexpr Generation getGeneration() const noexcept { return generation_; }

        constexpr bool isValid() const noexcept {
            return id_ != INVALID_ID && generation_ != INVALID_GENERATION;
        }

        constexpr bool operator==(const ResourceHandle& other) const noexcept {
            return id_ == other.id_ && generation_ == other.generation_;
        }

        constexpr bool operator!=(const ResourceHandle& other) const noexcept {
            return !(*this == other);
        }

        constexpr bool operator<(const ResourceHandle& other) const noexcept {
            if (id_ != other.id_) {
                return id_ < other.id_;
            }
            return generation_ < other.generation_;
        }

        // Hash support for containers
        std::size_t hash() const noexcept {
            return static_cast<std::size_t>(id_) |
                (static_cast<std::size_t>(generation_) << 32);
        }

        // String conversion for debugging
        std::string toString() const {
            if (!isValid()) {
                return "ResourceHandle::INVALID";
            }
            return "ResourceHandle(" + std::to_string(id_) + ":"
                + std::to_string(generation_) + ")";
        }

    private:
        HandleId id_;
        Generation generation_;

    };

    inline constexpr ResourceHandle INVALID_RESOURCE_HANDLE{};

} // namespace Simulacrum

inline std::ostream &operator<<(std::ostream &os, const Simulacrum::ResourceHandle& handle) {
    return os << handle.toString();
}

namespace std {
    template <> struct hash<Simulacrum::ResourceHandle> {
        std::size_t
        operator()(const Simulacrum::ResourceHandle& handle) const noexcept {
            return handle.hash();
        }
    };
}
