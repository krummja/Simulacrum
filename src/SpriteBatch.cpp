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
  {
    if (!initialized_)
    {
      spdlog::error("SpriteBatch::begin: not initialized");
      return;
    }

    if (recording_)
    {
      spdlog::warn("SpriteBatch::begin: already recording, calling end()");
      end();
    }

    write_ptr_ = write_ptr;
    max_vertices_ = max_vertices;
    texture_ = texture;
    sampler_ = sampler;

    texture_width_ = (texture_width > 0) ? texture_width : 1.0f;
    texture_height_ = (texture_height > 0) ? texture_height : 1.0f;

    sprite_count_ = 0;
    vertex_count_ = 0;
    recording_ = true;
  }

  void SpriteBatch::draw(
    float src_x, float src_y, float src_w, float src_h,
    float dst_x, float dst_y, float dst_w, float dst_h,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
  )
  {
    float u0 = src_x / texture_width_;
    float v0 = src_y / texture_height_;
    float u1 = (src_x + src_w) / texture_width_;
    float v1 = (src_y + src_h) / texture_height_;

    drawUV(u0, v0, u1, v1, dst_x, dst_y, dst_w, dst_h, r, g, b, a);
  }

  void SpriteBatch::drawUV(
    float u0, float v0, float u1, float v1,
    float dst_x, float dst_y, float dst_w, float dst_h,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
  )
  {
    if (!recording_)
    {
      return;
    }

    // Check capacity
    if (vertex_count_ + VERTICES_PER_SPRITE > max_vertices_)
    {
      spdlog::warn("SpriteBatch: vertex capacity exceeded");
      return;
    }

    if (sprite_count_ > MAX_SPRITES)
    {
      spdlog::warn("SpriteBatch: sprite capacity exceeded");
      return;
    }

    float x0 = dst_x;
    float y0 = dst_y;
    float x1 = dst_x + dst_w;
    float y1 = dst_y + dst_h;

    addQuad(x0, y0, x1, y1, u0, v0, u1, v1, r, g, b, a);
  }

  void SpriteBatch::addQuad(
    float x0, float y0, float x1, float y1,
    float u0, float v0, float u1, float v1,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
  )
  {
    if (!write_ptr_)
    {
      return;
    }

    SpriteVertex* v = write_ptr_ + vertex_count_;

    // Vertex 0: top-left
    v[0] = {x0, y0, u0, v0, r, g, b, a};
    // Vertex 1: top-right
    v[1] = {x1, y0, u1, v0, r, g, b, a};
    // Vertex 2: bottom-right
    v[2] = {x1, y1, u1, v1, r, g, b, a};
    // Vertex 3: bottom-left
    v[3] = {x0, y1, u0, v1, r, g, b, a};

    vertex_count_ += VERTICES_PER_SPRITE;
    ++sprite_count_;
  }

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
    if (!pass || !pipeline || !vertex_buffer)
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
