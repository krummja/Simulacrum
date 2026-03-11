#include "StateLoading.hpp"
#include "SimulacrumEngine.hpp"
#include "TextureManager.hpp"
#include "ThreadSystem.hpp"
#include "StateManager.hpp"
#include "GpuRenderer.hpp"
#include "GpuTypes.hpp"
#include "GpuTexture.hpp"
#include "UIManager.hpp"
#include "SpriteBatch.hpp"

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

    TextureManager& tex_mgr = TextureManager::Instance();

    auto* texture = tex_mgr.getGPUTextureData("tile_0815");

    sprite_batch_ = &gpu_renderer.getSpriteBatch();
    auto& vertex_pool = gpu_renderer.getSpriteVertexPool();
    auto* base_ptr = static_cast<SpriteVertex*>(vertex_pool.getMappedPtr());
    if (!base_ptr) return;

    sprite_batch_->begin(
      base_ptr, vertex_pool.getMaxVertices(),
      texture->texture->get(), gpu_renderer.getNearestSampler(),
      static_cast<float>(texture->width),
      static_cast<float>(texture->height)
    );
  }

  void LoadingState::renderGPUScene(
    GPURenderer& gpu_renderer,
    SDL_GPURenderPass* scene_pass,
    float interpolation_alpha
  )
  {}

  void LoadingState::renderGPUUI(GPURenderer& gpu_renderer, SDL_GPURenderPass* swapchain_pass)
  {

    // if (!swapchain_pass || image_commands_.empty()) return;
    // float ortho_matrix[16];

    // GPURenderer::createOrthoMatrix(
    //   0.0f, static_cast<float>(gpu_renderer.getViewportWidth()),
    //   static_cast<float>(gpu_renderer.getViewportHeight()), 0.0f,
    //   ortho_matrix
    // );

    // Bind UI sprite pipeline
    // SDL_BindGPUGraphicsPipeline(swapchain_pass, gpu_renderer.getUISpritePipeline());

    // Push view-projection matrix
    // gpu_renderer.pushViewProjection(swapchain_pass, ortho_matrix);

    // Bind vertex buffer
    // SDL_GPUBufferBinding vertex_binding{};
    // vertex_binding.buffer = gpu_renderer.getUIVertexPool().getGPUBuffer();
    // vertex_binding.offset = 0;
    // SDL_BindGPUVertexBuffers(swapchain_pass, 0, &vertex_binding, 1);

    // Bind index buffer
    // SDL_GPUBufferBinding index_binding{};
    // index_binding.buffer = sprite_batch_->getIndexBuffer();
    // index_binding.offset = 0;
    // SDL_BindGPUIndexBuffer(swapchain_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);


  }

} // namespace Simulacrum
