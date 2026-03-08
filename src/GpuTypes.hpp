#pragma once

#include <cstdint>
#include <cstddef>

namespace Simulacrum
{

  /// @brief Vertex format for textured sprites.
  struct SpriteVertex
  {
    float x, y;         // Position (8 bytes)
    float u, v;         // Texture coordinates (8 bytes)
    uint8_t r, g, b, a; // Color packed (4 bytes)
    // Total: 20 bytes per vertex
  };

  /// @brief Vertex format for colored primitives and particles.
  struct ColorVertex
  {
    float x, y;         // Position (8 bytes)
    uint8_t r, g, b, a; // Color packed (4 bytes)
    // Total: 12 bytes per vertex
  };

  /// @brief View-projection uniform buffer data.
  struct ViewProjectionUBO
  {
    float viewProjection[16]; // 4x4 matrix
  };

  /// @brief Composite uniform buffer data.
  struct CompositeUBO
  {
    float subPixelOffsetX;
    float subPixelOffsetY;
    float zoom;
    float _pad0;
  };

} // engine::render
