#pragma once

#include <SDL3/SDL_gpu.h>
#include <array>
#include <cstdint>

namespace Simulacrum
{

  /// @brief Pipeline type identifiers for render sorting.
  enum class PipelineType : uint8_t
  {
    SpriteOpaque = 0, // Depth write, no blend
    SpriteAlpha,      // Depth test, alpha blend
    Particle,         // No depth, additive/alpha blend
    Composite,        // Fullscreen quad composite
    Primitive,        // Colored primitives (UI backgrounds)
    Text,             // Text rendering
    COUNT
  };

  /// @brief Configuration for creating a graphics pipeline.
  /// Uses value semantics with embedded arrays for thread-safety.
  struct PipelineConfig
  {
    SDL_GPUShader* vertex_shader{ nullptr };
    SDL_GPUShader* fragment_shader{ nullptr };

    // Embedded vertex format data (value semantics, no pointers)
    std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffers{};
    std::array<SDL_GPUVertexAttribute, 4> vertex_attributes{};
    uint32_t vertex_buffer_count{ 0 };
    uint32_t vertex_attribute_count{ 0 };

    // Primitive type
    SDL_GPUPrimitiveType primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLELIST };

    // Depth/stencil state
    bool enable_depth_test{ false };
    bool enable_depth_write{ false };
    SDL_GPUCompareOp depth_compare_op{ SDL_GPU_COMPAREOP_LESS };

    // Blending state
    bool enable_blend{ true };
    SDL_GPUBlendFactor src_color_factor{ SDL_GPU_BLENDFACTOR_SRC_ALPHA };
    SDL_GPUBlendFactor dst_color_factor{ SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA };
    SDL_GPUBlendFactor src_alpha_factor{ SDL_GPU_BLENDFACTOR_ONE };
    SDL_GPUBlendFactor dst_alpha_factor{ SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA };
    SDL_GPUBlendOp color_blend_op{ SDL_GPU_BLENDOP_ADD };
    SDL_GPUBlendOp alpha_blend_op{ SDL_GPU_BLENDOP_ADD };

    // Color target format (typically swapchain format)
    SDL_GPUTextureFormat color_format{ SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM };

    // Rasterizer state
    SDL_GPUFillMode fill_mode{ SDL_GPU_FILLMODE_FILL };
    SDL_GPUCullMode cull_mode{ SDL_GPU_CULLMODE_NONE };
    SDL_GPUFrontFace front_face{ SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE };
  };

  class GPUPipeline
  {
  public:
    GPUPipeline() = default;
    ~GPUPipeline();

    // Move-only
    GPUPipeline(GPUPipeline&&) noexcept;
    GPUPipeline& operator=(GPUPipeline&&) noexcept;
    GPUPipeline(const GPUPipeline&) = delete;
    GPUPipeline& operator=(const GPUPipeline&) = delete;

    /// @brief Create a graphics pipeline from configuration.
    /// @param device GPU device
    /// @param config Pipeline configuration
    /// @return true on success
    bool create(SDL_GPUDevice* device, const PipelineConfig& config);

    /// @brief Release the pipeline
    void release();

    SDL_GPUGraphicsPipeline* get() const { return pipeline_; }
    bool isValid() const { return pipeline_ != nullptr; }

    /// @brief Create a standard sprite pipeline configuration.
    /// @param vert_shader Vertex shader
    /// @param frag_shader Fragment shader
    /// @param color_format Target color format
    /// @param alpha If true, enables alpha blending; if false, opaque
    static PipelineConfig createSpriteConfig(
      SDL_GPUShader* vert_shader,
      SDL_GPUShader* frag_shader,
      SDL_GPUTextureFormat color_format,
      bool alpha
    );

    static PipelineConfig createParticleConfig(
      SDL_GPUShader* vert_shader,
      SDL_GPUShader* frag_shader,
      SDL_GPUTextureFormat color_format
    );

    static PipelineConfig createPrimitiveConfig(
      SDL_GPUShader* vert_shader,
      SDL_GPUShader* frag_shader,
      SDL_GPUTextureFormat color_format
    );

    static PipelineConfig createCompositeConfig(
      SDL_GPUShader* vert_shader,
      SDL_GPUShader* frag_shader,
      SDL_GPUTextureFormat color_format
    );

  private:
    SDL_GPUGraphicsPipeline* pipeline_ = nullptr;
    SDL_GPUDevice* device_ = nullptr;

  };

} // namespace Simulacrum
