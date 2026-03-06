#pragma once

#include "unique_id.hpp"
#include <cstdint>
#include <format>
#include <functional>
#include <ostream>
#include <string>

namespace Simulacrum {

    enum class EntityKind : uint8_t {
        Player = 0,
        NPC = 1,
        DroppedItem = 2,
        Container = 3,
        Harvestable = 4,
        Projectile = 5,
        AreaEffect = 6,
        Prop = 7,
        Trigger = 8,
        StaticObstacle = 9,
        COUNT
    };

    enum class SimulationTier : uint8_t {
        Active = 0,
        Background = 1,
        Hibernated = 2
    };

    struct EntityHandle {
        using IDType = Simulacrum::UniqueID::IDType;
        using Generation = uint8_t;

        static constexpr IDType INVALID_ID = 0;
        static constexpr Generation INVALID_GENERATION = 0;

        IDType id{INVALID_ID};                     // 8 bytes: unique identifier
        EntityKind kind{EntityKind::NPC};          // 1 byte: entity type
        Generation generation{INVALID_GENERATION}; // 1 byte: stale reference detection
        uint16_t padding{0};                       // 2 bytes: alignment padding

        constexpr EntityHandle() noexcept = default;

        constexpr EntityHandle(
            IDType entity_id,
            EntityKind entity_kind,
            Generation generation
        ) noexcept
            : id(entity_id)
            , kind(entity_kind)
            , generation(generation)
            , padding(0)
        {}

        [[nodiscard]] constexpr bool isValid() const noexcept {
            return id != INVALID_ID && generation != INVALID_GENERATION;
        }

        [[nodiscard]] constexpr IDType getId() const noexcept { return id; }
        [[nodiscard]] constexpr EntityKind getKind() const noexcept { return kind; }
        [[nodiscard]] constexpr Generation getGeneration() const noexcept { return generation; }

        [[nodiscard]] constexpr bool
        operator==(const EntityHandle& other) const noexcept {
            return id == other.id && generation == other.generation && kind == other.kind;
        }

        [[nodiscard]] constexpr bool
        operator!=(const EntityHandle& other) const noexcept {
            return !(*this == other);
        }

        [[nodiscard]] constexpr bool
        operator<(const EntityHandle& other) const noexcept {
            if (id != other.id) return id < other.id;
            if (generation != other.generation) return generation < other.generation;
            return static_cast<uint8_t>(kind) < static_cast<uint8_t>(other.kind);
        }

        [[nodiscard]] std::size_t hash() const noexcept {
            std::size_t h = static_cast<std::size_t>(id);
            h ^= static_cast<std::size_t>(kind) << 48;
            h ^= static_cast<std::size_t>(generation) << 56;
            return h;
        }

        [[nodiscard]] std::string toString() const {
            if (!isValid()) {
                return "EntityHandle::INVALID";
            }
            return std::format("EntityHandle({}:{}:{})", id, kind, generation);
        }
    };

    // Static assertion to verify handle size
    // Note: Due to 8-byte alignment of uint64_t id, actual size is 16 bytes:
    // - id (8 bytes) + kind (1) + generation (1) + padding (2) + 4 bytes end padding = 16
    static_assert(sizeof(EntityHandle) == 16, "EntityHandle should be 16 bytes (8-byte aligned)");

    inline constexpr EntityHandle INVALID_ENTITY_HANDLE{};

    inline std::ostream& operator<<(std::ostream& os, const EntityHandle& handle) {
        return os << handle.toString();
    }

} // namespace Simulacrum

namespace std {
    template <>
    struct hash<Simulacrum::EntityHandle> {
        std::size_t operator()(const Simulacrum::EntityHandle& handle) const noexcept {
            return handle.hash();
        }
    };
}
