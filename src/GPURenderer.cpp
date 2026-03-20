#include "GPURenderer.hpp"
#include "ResourcePath.hpp"
#include "GPUShaderManager.hpp"
#include "Common.hpp"

#include <cstring>
#include <format>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace Simulacrum
{
  bool GPURenderer::init()
  {
    if (is_initialized_)
    {
      spdlog::warn("GPURenderer already initialized");
      return true;
    }

    auto& gpu_device = GPUDevice::Instance();

    device_ = gpu_device.get();
    window_ = gpu_device.getWindow();

    int w = 0;
    int h = 0;

    SDL_GetWindowSize(window_, &w, &h);
    viewport_width_ = static_cast<Uint32>(w);
    viewport_height_ = static_cast<Uint32>(h);

    if (!GPUShaderManager::Instance().init(device_))
    {
      spdlog::error("GPURenderer: failed to initialize shader manager.");
      return false;
    }

    // Create pipeline

    /// Shaders
    SDL_GPUShader* vertex_shader = GPUShaderManager::Instance().getShader("sprite.vert");
    SDL_GPUShader* fragment_shader = GPUShaderManager::Instance().getShader("sprite.frag");

    /// Swapchain Format
    SDL_GPUTextureFormat swapchain_format = SDL_GetGPUSwapchainTextureFormat(device_, window_);

    /// Vertex Input State
    SDL_GPUVertexInputState vertex_input_state{};

    /// - Vertex Buffer Descriptions
    std::array<SDL_GPUVertexBufferDescription, 1> vertex_buffers{};

    vertex_buffers[0].slot = 0;
    vertex_buffers[0].pitch = sizeof(PositionTextureVertex);
    vertex_buffers[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffers[0].instance_step_rate = 0;

    /// - Vertex Attributes
    std::array<SDL_GPUVertexAttribute, 4> vertex_attributes{};

    vertex_attributes[0].location = 0;
    vertex_attributes[0].buffer_slot = 0;
    vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attributes[0].offset = 0;

    vertex_attributes[1].location = 1;
    vertex_attributes[1].buffer_slot = 0;
    vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    vertex_attributes[1].offset = sizeof(float) * 3;

    vertex_input_state.num_vertex_buffers = 1;
    vertex_input_state.vertex_buffer_descriptions = vertex_buffers.data();
    vertex_input_state.num_vertex_attributes = 2;
    vertex_input_state.vertex_attributes = vertex_attributes.data();

    /// Target Info
    SDL_GPUColorTargetDescription swapchain_target{};
    swapchain_target.format = swapchain_format;

    // Compose Pipeline Info
    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.vertex_shader = vertex_shader;
    pipeline_info.fragment_shader = fragment_shader;
    pipeline_info.vertex_input_state = vertex_input_state;
    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_info.target_info.color_target_descriptions = &swapchain_target;
    pipeline_ = SDL_CreateGPUGraphicsPipeline(device_, &pipeline_info);

    // Vertex Buffer
    SDL_GPUBufferCreateInfo vertex_buffer_info{};
    vertex_buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer_info.size = sizeof(PositionTextureVertex) * 4;
    vertex_buffer_ = SDL_CreateGPUBuffer(device_, &vertex_buffer_info);

    // Index Buffer
    SDL_GPUBufferCreateInfo index_buffer_info{};
    index_buffer_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer_info.size = sizeof(Uint16) * 6;
    index_buffer_ = SDL_CreateGPUBuffer(device_, &index_buffer_info);

    // Test Texture
    SDL_Surface* img_data = LoadPNGTexture("tile_0000.png", 4);

    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.type = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.width = img_data->w;
    texture_info.height = img_data->h;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    test_texture_ = SDL_CreateGPUTexture(device_, &texture_info);

    // Sampler
    SDL_GPUSamplerCreateInfo sampler_info{};
    sampler_info.min_filter = SDL_GPU_FILTER_NEAREST;
    sampler_info.mag_filter = SDL_GPU_FILTER_NEAREST;
    sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_ = SDL_CreateGPUSampler(device_, &sampler_info);

    // Transfer Buffer
    SDL_GPUTransferBufferCreateInfo transfer_buffer_info{};
    transfer_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer_info.size = (sizeof(PositionTextureVertex) * 4) + (sizeof(Uint16) * 6);
    transfer_buffer_ = SDL_CreateGPUTransferBuffer(device_, &transfer_buffer_info);

    PositionTextureVertex* transfer_data = static_cast<PositionTextureVertex*>(
      SDL_MapGPUTransferBuffer(device_, transfer_buffer_, false)
    );

    transfer_data[0] = { -1,  1, 0, 0, 0 };
    transfer_data[1] = {  1,  1, 0, 4, 0 };
    transfer_data[2] = {  1, -1, 0, 4, 4 };
    transfer_data[3] = { -1, -1, 0, 0, 4 };

    Uint16* index_data = (Uint16*) &transfer_data[4];

    index_data[0] = 0;
    index_data[1] = 1;
    index_data[2] = 2;
    index_data[3] = 0;
    index_data[4] = 2;
    index_data[5] = 3;

    SDL_UnmapGPUTransferBuffer(device_, transfer_buffer_);

    is_initialized_ = true;
    return true;
  }

  void GPURenderer::shutdown()
  {

  }

  void GPURenderer::beginFrame()
  {
    // Acquire command buffer

    // Acquire swapchain texture

    // Begin Copy Pass
  }

  SDL_GPURenderPass* GPURenderer::beginScenePass()
  {
    return nullptr;
  }

  SDL_GPURenderPass* GPURenderer::beginSwapchainPass()
  {
    return nullptr;
  }

  void GPURenderer::endFrame()
  {

  }
}
