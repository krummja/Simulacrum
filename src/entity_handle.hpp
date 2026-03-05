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
        using IDType = Simulacrum::UniqueId::IDType;
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
    };

} // namespace Simulacrum
