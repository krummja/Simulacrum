#include "GpuPipeline.hpp"
#include <spdlog/spdlog.h>

namespace Simulacrum
{

  GPUPipeline::~GPUPipeline()
  {
    release();
  }

  GPUPipeline::GPUPipeline(GPUPipeline&& other) noexcept
    : pipeline_(other.pipeline_)
    , device_(other.device_)
  {
    other.pipeline_ = nullptr;
    other.device_ = nullptr;
  }

  GPUPipeline& GPUPipeline::operator=(GPUPipeline&& other) noexcept
  {
    if (this != &other)
    {
      release();

      pipeline_ = other.pipeline_;
      device_ = other.device_;

      other.pipeline_ = nullptr;
      other.device_ = nullptr;
    }
    return *this;
  }

  bool GPUPipeline::create(SDL_GPUDevice* device, const PipelineConfig& config)
  {
    if (!device)
    {
      spdlog::error("GPUPipeline::create: null device");
      return false;
    }

    if (!config.vertex_shader || !config.fragment_shader)
    {
      spdlog::error("GPUPipeline::create: missing shaders");
      return false;
    }

    release();
    device_ = device;

    // Build color target description
    SDL_GPUColorTargetDescription color_target{};
    color_target.format = config.color_format;

    if (config.enable_blend)
    {
      color_target.blend_state.enable_blend = true;
      color_target.blend_state.src_color_blendfactor = config.src_color_factor;
      color_target.blend_state.dst_color_blendfactor = config.dst_color_factor;
      color_target.blend_state.color_blend_op = config.color_blend_op;
      color_target.blend_state.src_alpha_blendfactor = config.src_alpha_factor;
      color_target.blend_state.dst_alpha_blendfactor = config.dst_alpha_factor;
      color_target.blend_state.alpha_blend_op = config.alpha_blend_op;
      color_target.blend_state.color_write_mask =
        SDL_GPU_COLORCOMPONENT_R |
        SDL_GPU_COLORCOMPONENT_G |
        SDL_GPU_COLORCOMPONENT_B |
        SDL_GPU_COLORCOMPONENT_A;
    }

    else
    {
      color_target.blend_state.enable_blend = false;
      color_target.blend_state.color_write_mask =
        SDL_GPU_COLORCOMPONENT_R |
        SDL_GPU_COLORCOMPONENT_G |
        SDL_GPU_COLORCOMPONENT_B |
        SDL_GPU_COLORCOMPONENT_A;
    }


    // Build rasterizer state
    SDL_GPURasterizerState rasterizer{};
    rasterizer.fill_mode = config.fill_mode;
    rasterizer.cull_mode = config.cull_mode;
    rasterizer.front_face = config.front_face;
    rasterizer.enable_depth_bias = false;
    rasterizer.enable_depth_clip = true;

    // Build depth/stencil state
    SDL_GPUDepthStencilState depth_stencil{};
    depth_stencil.enable_depth_test = config.enable_depth_test;
    depth_stencil.enable_depth_write = config.enable_depth_write;
    depth_stencil.compare_op = config.depth_compare_op;
    depth_stencil.enable_stencil_test = false;

    // Build vertex input state from embedded config arrays
    SDL_GPUVertexInputState vertex_input{};
    vertex_input.num_vertex_buffers = config.vertex_buffer_count;
    vertex_input.vertex_buffer_descriptions = config.vertex_buffer_count > 0
      ? config.vertex_buffers.data() : nullptr;
    vertex_input.num_vertex_attributes = config.vertex_attribute_count;
    vertex_input.vertex_attributes = config.vertex_attribute_count > 0
      ? config.vertex_attributes.data() : nullptr;

    // Build pipeline create info
    SDL_GPUGraphicsPipelineCreateInfo create_info{};
    create_info.vertex_shader = config.vertex_shader;
    create_info.fragment_shader = config.fragment_shader;
    create_info.vertex_input_state = vertex_input;
    create_info.primitive_type = config.primitive_type;
    create_info.rasterizer_state = rasterizer;
    create_info.depth_stencil_state = depth_stencil;
    create_info.target_info.num_color_targets = 1;
    create_info.target_info.color_target_descriptions = &color_target;
    create_info.target_info.has_depth_stencil_target = false;

    pipeline_ = SDL_CreateGPUGraphicsPipeline(device, &create_info);

    if (!pipeline_)
    {
      spdlog::critical("Failed to create GPU pipeline: {}", SDL_GetError());
      return false;
    }

    return true;
  }

  void GPUPipeline::release()
  {
    if (pipeline_ && device_)
    {
      SDL_ReleaseGPUGraphicsPipeline(device_, pipeline_);
      pipeline_ = nullptr;
    }
  }

  PipelineConfig GPUPipeline::createSpriteConfig(
    SDL_GPUShader* vert_shader,
    SDL_GPUShader* frag_shader,
    SDL_GPUTextureFormat color_format,
    bool alpha
  )
  {
    PipelineConfig config{};

    config.vertex_shader = vert_shader;
    config.fragment_shader = frag_shader;
    config.color_format = color_format;
    config.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // Vertex buffer: position(vec2) + texcoord(vec2) + color(rgba8) = 20 bytes
    config.vertex_buffers[0].slot = 0;
    config.vertex_buffers[0].pitch = (sizeof(float) * 4) + (sizeof(uint8_t) * 4);
    config.vertex_buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    config.vertex_buffers[0].instance_step_rate = 0;
    config.vertex_buffer_count = 1;

    // Vertex attributes: position, texcoord, color

    // layout(location = 0) in vec2 inPosition
    config.vertex_attributes[0].location = 0;
    config.vertex_attributes[0].buffer_slot = 0;
    config.vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    config.vertex_attributes[0].offset = 0;

    // layout(location = 1) in vec2 inTexCoord
    config.vertex_attributes[1].location = 1;
    config.vertex_attributes[1].buffer_slot = 0;
    config.vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    config.vertex_attributes[1].offset = sizeof(float) * 2;

    // layout(location = 2) in vec4 inColor
    config.vertex_attributes[2].location = 2;
    config.vertex_attributes[2].buffer_slot = 0;
    config.vertex_attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
    config.vertex_attributes[2].offset = sizeof(float) * 4;

    config.vertex_attribute_count = 3;

    if (alpha)
    {
      config.enable_blend = true;
      config.src_color_factor = SDL_GPU_BLENDFACTOR_ONE;
      config.dst_color_factor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
      config.color_blend_op = SDL_GPU_BLENDOP_ADD;
      config.src_alpha_factor = SDL_GPU_BLENDFACTOR_ONE;
      config.dst_alpha_factor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
      config.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    }

    else
    {
      config.enable_blend = false;
    }

    return config;
  }

  PipelineConfig GPUPipeline::createPrimitiveConfig(
    SDL_GPUShader* vert_shader,
    SDL_GPUShader* frag_shader,
    SDL_GPUTextureFormat color_format
  )
  {
    PipelineConfig config{};

    config.vertex_shader = vert_shader;
    config.fragment_shader = frag_shader;
    config.color_format = color_format;
    config.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // Vertex buffer: position(vec2) + color(rgba8) = 12 bytes
    config.vertex_buffers[0].slot = 0;
    config.vertex_buffers[0].pitch = (sizeof(float) * 2) + (sizeof(uint8_t) * 4);
    config.vertex_buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    config.vertex_buffers[0].instance_step_rate = 0;

    config.vertex_buffer_count = 1;

    // Vertex attributes: position, color

    // layout(location = 0) in vec2 inPosition
    config.vertex_attributes[0].location = 0;
    config.vertex_attributes[0].buffer_slot = 0;
    config.vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    config.vertex_attributes[0].offset = 0;

    // layout(location = 1) in vec4 inColor
    config.vertex_attributes[1].location = 1;
    config.vertex_attributes[1].buffer_slot = 0;
    config.vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
    config.vertex_attributes[1].offset = sizeof(float) * 2;

    config.vertex_attribute_count = 2;

    config.enable_blend = true;
    config.src_color_factor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    config.dst_color_factor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

    return config;
  }

  PipelineConfig GPUPipeline::createCompositeConfig(
    SDL_GPUShader* vert_shader,
    SDL_GPUShader* frag_shader,
    SDL_GPUTextureFormat color_format
  )
  {
    PipelineConfig config{};

    config.vertex_shader = vert_shader;
    config.fragment_shader = frag_shader;
    config.color_format = color_format;
    config.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // No vertex input - fullscreen triangle uses gl_VertexIndex
    config.vertex_buffer_count = 0;
    config.vertex_attribute_count = 0;

    config.enable_blend = false;

    return config;
  }

} // namespace Simulacrum
