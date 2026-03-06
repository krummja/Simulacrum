#include "SpriteBatch.hpp"
#include "GpuTypes.hpp"
#include "GpuTransferBuffer.hpp"
#include <spdlog/spdlog.h>
#include <cstring>

namespace Simulacrum
{

  bool SpriteBatch::init(SDL_GPUDevice* device)
  {
    if (!device)
    {
      spdlog::error("SpriteBatch::init: null device");
      return false;
    }

    device_ = device;

    // Build index buffer (all quads share same index pattern)
    // Use 32-bit indices to support >10k sprites (4k needs ~20k sprites)
    std::vector<uint32_t> indices;
    indices.reserve(MAX_INDICES);

    for (size_t i = 0; i < MAX_SPRITES; ++i)
    {
      uint32_t base_vertex = static_cast<uint32_t>(i * VERTICES_PER_SPRITE);
      // Two triangles per quad: 0-1-2, 2-3-0
      indices.push_back(base_vertex + 0);
      indices.push_back(base_vertex + 1);
      indices.push_back(base_vertex + 2);
      indices.push_back(base_vertex + 2);
      indices.push_back(base_vertex + 3);
      indices.push_back(base_vertex + 0);
    }

    // Create index buffer
    uint32_t index_buffer_size = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));
    index_buffer_ = GPUBuffer(device, SDL_GPU_BUFFERUSAGE_INDEX, index_buffer_size);

    if (!index_buffer_.isValid())
    {
      spdlog::error("SpriteBatch: failed to create index buffer");
      return false;
    }

    // Upload indices via transfer buffer
    GPUTransferBuffer transfer_buffer(device, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, index_buffer_size);
    if (!transfer_buffer.isValid())
    {
      spdlog::error("SpriteBatch: failed to create transfer buffer for indices");
      return false;
    }

    void* mapped = transfer_buffer.map(false);
    if (!mapped)
    {
      spdlog::error("SpriteBatch: failed to map transfer buffer");
      return false;
    }

    std::memcpy(mapped, indices.data(), index_buffer_size);
    transfer_buffer.unmap();

    // Upload to GPU (need a one-time copy pass)
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd)
    {
      spdlog::error("SpriteBatch: failed to acquire command buffer: {}", SDL_GetError());
      return false;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    if (!copy_pass)
    {
      spdlog::error("SpriteBatch: failed to begin copy pass: {}", SDL_GetError());
      SDL_CancelGPUCommandBuffer(cmd);
      return false;
    }

    SDL_GPUTransferBufferLocation src = transfer_buffer.asLocation(0);
    SDL_GPUBufferRegion dst{};
    dst.buffer = index_buffer_.get();
    dst.offset = 0;
    dst.size = index_buffer_size;

    SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);

    initialized_ = true;
    spdlog::info(
      "SpriteBatch initialized: max {} sprites, {} KB index buffer",
      MAX_SPRITES,
      index_buffer_size / 1024
    );

    return true;
  }

  void SpriteBatch::shutdown()
  {
    index_buffer_ = GPUBuffer();
    device_ = nullptr;
    initialized_ = false;
  }

  SpriteBatch::SpriteBatch(SpriteBatch&& other) noexcept
    : device_(other.device_)
    , texture_(other.texture_)
    , sampler_(other.sampler_)
    , index_buffer_(std::move(other.index_buffer_))
    , write_ptr_(other.write_ptr_)
    , max_vertices_(other.max_vertices_)
    , texture_width_(other.texture_width_)
    , texture_height_(other.texture_height_)
    , sprite_count_(other.sprite_count_)
    , vertex_count_(other.vertex_count_)
    , recording_(other.recording_)
    , initialized_(other.initialized_)
  {
    other.device_ = nullptr;
    other.texture_ = nullptr;
    other.sampler_ = nullptr;
    other.write_ptr_ = 0;
    other.max_vertices_ = 0;
    other.texture_width_ = 0;
    other.texture_height_ = 0;
    other.sprite_count_ = 0;
    other.vertex_count_ = 0;
    other.recording_ = false;
    other.initialized_ = false;
  }

  SpriteBatch& SpriteBatch::operator=(SpriteBatch&& other) noexcept
  {
    if (this != &other)
    {
      shutdown();
    }

    device_ = other.device_;
    texture_ = other.texture_;
    sampler_ = other.sampler_;
    index_buffer_ = std::move(other.index_buffer_);
    write_ptr_ = other.write_ptr_;
    max_vertices_ = other.max_vertices_;
    texture_width_ = other.texture_width_;
    texture_height_ = other.texture_height_;
    sprite_count_ = other.sprite_count_;
    vertex_count_ = other.vertex_count_;
    recording_ = other.recording_;
    initialized_ = other.initialized_;

    other.device_ = nullptr;
    other.texture_ = nullptr;
    other.sampler_ = nullptr;
    other.write_ptr_ = 0;
    other.max_vertices_ = 0;
    other.texture_width_ = 0;
    other.texture_height_ = 0;
    other.sprite_count_ = 0;
    other.vertex_count_ = 0;
    other.recording_ = false;
    other.initialized_ = false;

    return *this;
  }

  void SpriteBatch::begin(
    SpriteVertex* write_ptr,
    size_t max_vertices,
    SDL_GPUTexture* texture,
    SDL_GPUSampler* sampler,
    float texture_width,
    float texture_height
  )
  {}

  void SpriteBatch::draw(
    float srcX, float srcY, float srcW, float srcH,
    float dstX, float dstY, float dstW, float dstH,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
  )
  {}

  void SpriteBatch::drawUV(
    float srcX, float srcY, float srcW, float srcH,
    float dstX, float dstY, float dstW, float dstH,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
  )
  {}

  void SpriteBatch::addQuad(
    float x0, float y0, float x1, float y1,
    float u0, float v0, float u1, float v1,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
  )
  {}

  size_t SpriteBatch::end()
  {
    if (!recording_)
    {
      return 0;
    }

    size_t result = vertex_count_;
    write_ptr_ = nullptr;
    recording_ = false;

    return result;
  }

  void SpriteBatch::render(
    SDL_GPURenderPass* pass,
    SDL_GPUGraphicsPipeline* pipeline,
    SDL_GPUBuffer* vertex_buffer
  )
  {
    if (!pass || !pipeline || !vertex_buffer || sprite_count_ == 0)
    {
      return;
    }

    // Bind pipeline
    SDL_BindGPUGraphicsPipeline(pass, pipeline);

    // Bind texture and sampler, if they exist
    if (texture_ && sampler_)
    {
      SDL_GPUTextureSamplerBinding tex_sampler{};
      tex_sampler.texture = texture_;
      tex_sampler.sampler = sampler_;
      SDL_BindGPUFragmentSamplers(pass, 0, &tex_sampler, 1);
    }

    // Bind vertex buffer
    SDL_GPUBufferBinding vertex_binding{};
    vertex_binding.buffer = vertex_buffer;
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);

    // Bind index buffer
    SDL_GPUBufferBinding index_binding{};
    index_binding.buffer = index_buffer_.get();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    // Issue indexed draw call
    uint32_t index_count = static_cast<uint32_t>(sprite_count_ * INDICES_PER_SPRITE);
    SDL_DrawGPUIndexedPrimitives(pass, index_count, 1, 0, 0, 0);
  }

} // namespace Simulacrum
