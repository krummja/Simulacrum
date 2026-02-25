#pragma once

#include <cstdint>
#include <cstddef>

namespace engine::render {

    /// @brief Vertex format for textured sprites.
    struct SpriteVertex {
        float x, y;         // Position (8 bytes)
        float u, v;         // Texture coordinates (8 bytes)
        uint8_t r, g, b, a; // Color packed (4 bytes)
        // Total: 20 bytes per vertex
    };

    /// @brief Vertex format for colored primitives and particles.
    struct ColorVertex {
        float x, y;         // Position (8 bytes)
        uint8_t r, g, b, a; // Color packed (4 bytes)
        // Total: 12 bytes per vertex
    };

    /// @brief View-projection uniform buffer data.
    struct ViewProjectionUBO {
        float viewProjection[16]; // 4x4 matrix
    };

    /// @brief Composite uniform buffer data.
    /// Used for fullscreen composite pass with day/night lighting.
    struct CompositeUBO {
        float subPixelOffsetX;
        float subPixelOffsetY;
        float zoom;
        float _pad0;
        // Day/night ambient lighting (0-1 range)
        float ambientR;
        float ambientG;
        float ambientB;
        float ambientAlpha; // Blend strength: 0 = no tint, 1 = full tint
    };

} // engine::render
