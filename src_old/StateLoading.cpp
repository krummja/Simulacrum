#include "StateLoading.hpp"
#include "SimulacrumEngine.hpp"
#include "TextureManager.hpp"
#include "ThreadSystem.hpp"
#include "StateManager.hpp"
#include "GpuRenderer.hpp"
#include "GpuTypes.hpp"
#include "GpuTexture.hpp"
#include "UIManager.hpp"

#include <format>

namespace Simulacrum
{
  void LoadingState::configure(const std::string& target_state_name)
  {
    target_state_name_ = target_state_name;
  }

  bool LoadingState::enter()
  {
    spdlog::debug("Entering Loading State");
    return true;
  }

  void LoadingState::update([[maybe_unused]] float delta_time) {}

  void LoadingState::render(SDL_Renderer* renderer, float /* interpolation_alpha */) {}

  void LoadingState::handleInput() {}

  void LoadingState::exit() {}

  std::string LoadingState::getName() const { return "Loading State"; }

  void LoadingState::recordGPUVertices(GPURenderer& gpu_renderer, [[maybe_unused]] float interpolation_alpha)
  {
    SimulacrumEngine& engine = SimulacrumEngine::Instance();
    int current_width = engine.getLogicalWidth();
    int current_height = engine.getLogicalHeight();

    image_commands_.clear();

    TextureManager& tex_mgr = TextureManager::Instance();

    auto& vertex_pool = gpu_renderer.getSpriteVertexPool();
    auto* base_ptr = static_cast<SpriteVertex*>(vertex_pool.getMappedPtr());
    if (!base_ptr) return;

    uint32_t vertex_offset = 0;

    auto addTexture = [&](const char* texture_id, int x, int y, int w, int h)
      {
        const TextureData* tex_data = tex_mgr.getGPUTextureData(texture_id);
        if (!tex_data || !tex_data->texture) return;

        SpriteVertex* v = base_ptr + vertex_offset;
        float sx = static_cast<float>(x);
        float sy = static_cast<float>(y);
        float sw = static_cast<float>(w);
        float sh = static_cast<float>(h);

        // //       x        y         u     v     r    g    b    a
        // v[0] = { sx,      sy,       0.0f, 0.0f, 255, 255, 255, 255 };
        // v[1] = { sx + sw, sy,       1.0f, 0.0f, 255, 255, 255, 255 };
        // v[2] = { sx + sw, sy + sh,  1.0f, 1.0f, 255, 255, 255, 255 };
        // v[3] = { sx,      sy + sh,  0.0f, 1.0f, 255, 255, 255, 255 };

        // Triangle 1
        v[0] = { sx,        sy,       0.0f, 0.0f,   255, 255, 255, 255 };
        v[1] = { sx + sw,   sy,       1.0f, 0.0f,   255, 255, 255, 255 };
        v[2] = { sx + sw,   sy + sh,  1.0f, 1.0f,   255, 255, 255, 255 };
        // Triangle 2
        v[3] = { sx,        sy,       0.0f, 0.0f,   255, 255, 255, 255 };
        v[4] = { sx + sw,   sy + sh,  1.0f, 1.0f,   255, 255, 255, 255 };
        v[5] = { sx,        sy + sh,  0.0f, 1.0f,   255, 255, 255, 255 };

        GPUDrawCommand cmd;
        cmd.texture = tex_data->texture->get();
        cmd.vertex_offset = vertex_offset;
        cmd.vertex_count = 4;
        image_commands_.push_back(cmd);
        vertex_offset += 4;
      };

    addTexture("tile_0815", 10, 10, 16, 16);

    vertex_pool.setWrittenVertexCount(vertex_offset);
  }

  void LoadingState::renderGPUScene(
    GPURenderer& gpu_renderer,
    SDL_GPURenderPass* scene_pass,
    float interpolation_alpha
  )
  {}

  void LoadingState::renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass)
  {
    if (!swapchain_pass || image_commands_.empty()) return;
    float ortho_matrix[16];

    GPURenderer::createOrthoMatrix(
      0.0f, static_cast<float>(gpu_renderer.getViewportWidth()),
      static_cast<float>(gpu_renderer.getViewportHeight()), 0.0f,
      ortho_matrix
    );

    // Bind UI sprite pipeline
    SDL_BindGPUGraphicsPipeline(swapchain_pass, gpu_renderer.getUISpritePipeline());

    // Push view-projection matrix
    gpu_renderer.pushViewProjection(swapchain_pass, ortho_matrix);

    // Bind vertex buffer
    SDL_GPUBufferBinding vertex_binding{};
    vertex_binding.buffer = gpu_renderer.getUIVertexPool().getGPUBuffer();
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(swapchain_pass, 0, &vertex_binding, 1);

    // Bind index buffer
    auto& batch = gpu_renderer.getSpriteBatch();
    SDL_GPUBufferBinding index_binding{};
    index_binding.buffer = batch.getIndexBuffer();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(swapchain_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    // Draw each texture
    for (const auto& cmd : image_commands_)
    {
      SDL_GPUTextureSamplerBinding tex_sampler{};
      tex_sampler.texture = cmd.texture;
      tex_sampler.sampler = gpu_renderer.getLinearSampler();
      SDL_BindGPUFragmentSamplers(swapchain_pass, 0, &tex_sampler, 1);

      uint32_t total_vertices = std::accumulate(
          image_commands_.begin(), image_commands_.end(), 0u,
          [](uint32_t sum, const auto& cmd)
          {
            return sum + cmd.vertex_count;
          }
        );

        if (total_vertices > 0)
        {
          SDL_DrawGPUPrimitives(swapchain_pass, total_vertices, 1, 0, 0);
        }

      // uint32_t first_index = (cmd.vertex_offset / 4) * 6;
      // SDL_DrawGPUIndexedPrimitives(swapchain_pass, 6, 1, first_index, 0, 0);
    }
  }

} // namespace Simulacrum
