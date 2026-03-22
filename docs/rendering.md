# GPU Rendering

## Separate Classes

### GameEngine

```cpp
void GameEngine::render()
{
    auto& gpu_renderer = GPURenderer::Instance();

    // Acquires the command buffer
    // Acquires the swapchain texture
    // Calls beginFrame() on all vertex pools
    // Begins the copy pass
    gpu_renderer.beginFrame();

    state_manager_->recordGPUVertices(gpu_renderer);

    // Ends the copy pass
    // Calls endFrame() on all vertex pools
    // calls upload(copy_pass) on all vertex pools
    // Begins scene render pass
    SDL_GPURenderPass* scene_pass = gpu_renderer.beginScenePass();

    // Scene rendering steps
    if (scenePass)
    {
        state_manager_->renderGPUScene(gpu_renderer, scene_pass);
    }

    // Ends the scene render pass
    // Begins the swapchain render pass
    SDL_GPURenderPass* swapchain_pass = gpu_renderer.beginSwapchainPass();

    // Swapchain rendering steps (composite and UI)
    if (swapchain_pass)
    {
        gpu_renderer.renderComposite(swapchain_pass);
    }

    if (swapchain_pass)
    {
        state_manager_->renderGPUUI(gpu_renderer, swapchain_pass);
    }
}

void GameEngine::present()
{
    // Ends the swapchain render pass
    // Ends any active copy pass
    // Submits the command buffer
    GPURenderer::Instance().endFrame();
}
```

### GPURenderer

```cpp
void GPURenderer::beginFrame()
{
    // Acquire command buffer
    command_buffer_ = SDL_AcquireGPUCommandBuffer(device_);

    // Acquire swapchain texture
    SDL_WaitAndAcquireGPUSwapchainTexture(...);

    if (!swapchain_texture_)
    {
        SDL_CancelGPUCommandBuffer(command_buffer_);
        return;
    }

    // Begin vertex pool frames (maps transfer buffers)
    // Calling begin frame creates the write pointers for each pool
    // Use these pointers to record vertex information for the render passes
    sprite_vertex_pool_.beginFrame();
    entity_vertex_pool_.beginFrame();
    ui_vertex_pool_.beginFrame();

    // Begin copy pass for uploads
    copy_pass_ = SDL_BeginGPUCopyPass(command_buffer_);
}

SDL_GPURenderPass* GPURenderer::beginScenePass()
{
    // End copy pass
    if (copy_pass_)
    {
        // Process pending texture uploads
        TextureManager::Instance().processPendingUploads(copy_pass_);

        // End vertex pool frames (unmaps buffers for upload)
        sprite_vertex_pool_.endFrame(...);

        // End entity vertex pool
        entity_vertex_pool_.endFrame(...);

        // Upload vertex data
        sprite_vertex_pool_.upload(copy_pass_);
        entity_vertex_pool_.upload(copy_pass_);
        ui_vertex_pool_.upload(copy_pass_);

        SDL_EndGPUCopyPass(copy_pass_);
    }

    // Begin scene render pass
    current_pass_ = SDL_BeginGPURenderPass(...);

    return current_pass_;
}

SDL_GPURenderPass* GPURenderer::beginSwapchainPass()
{
    // End scene pass
    if (current_pass_)
    {
        SDL_EndGPURenderPass(current_pass_);
    }

    // Swapchain already acquired in beginFrame()
    if (!swapchain_texture_)
    {
        // Window minimized or not visible
        return nullptr;
    }

    // Begin swapchain render pass
    current_pass_ = SDL_BeginGPURenderPass(...);

    // Render swapchain pass pointer
    return current_pass_;
}

void GPURenderer::endFrame()
{
    // End active render pass
    if (current_pass_)
    {
        SDL_EndGPURenderPass(current_pass_);
    }

    // End copy pass if still active
    if (copy_pass_)
    {
        SDL_EndGPUCopyPass(copy_pass_);
    }

    // Submit command buffer
    SDL_SubmitGPUCommandBuffer(command_buffer_);
}
```


## Combined View

```cpp
void GameEngine::render()
{
    auto& gpu_renderer = GPURenderer::Instance();

    // BEGIN FRAME -- START
    command_buffer_ = SDL_AcquireGPUCommandBuffer(device_);

    SDL_WaitAndAcquireSwapchainTexture(..., swapchain_texture_, ...);

    if (!swapchain_texture_)
    {
        SDL_CancelGPUCommandBuffer(command_buffer_);
        return;
    }

    sprite_vertex_pool_.beginFrame();

    copy_pass_ = SDL_BeginGPUCopyPass(command_buffer_);
    // BEGIN FRAME -- END


    state_manager_->recordGPUVertices(gpu_renderer);


    // BEGIN SCENE PASS -- START
    if (copy_pass_)
    {
        TextureManager::Instance().processPendingUploads(copy_pass_);

        sprite_vertex_pool_.endFrame(...);
        entity_vertex_pool_.endFrame(...);

        sprite_vertex_pool_.upload(copy_pass_);
        entity_vertex_pool_.upload(copy_pass_);
        ui_vertex_pool_.upload(copy_pass_);

        SDL_EndGPUCopyPass(copy_pass_);
    }

    current_pass_ = SDL_BeginGPURenderPass(...);
    // BEGIN SCENE PASS -- END


    // BEGIN SWAPCHAIN PASS -- START
    if (current_pass_)
    {
        SDL_EndGPURenderPass(current_pass_);
    }

    if (!swapchain_texture_)
    {
        return nullptr;
    }

    current_pass_ = SDL_BeginGPURenderPass(...);
    // BEGIN SWAPCHAIN PASS -- END


    if (swapchain_pass)
    {
        gpu_renderer.renderComposite(swapchain_pass);
    }

    if (swapchain_pass)
    {
        state_manager_->renderGPUUI(gpu_renderer, swapchain_pass);
    }
}

void GPURenderer::endFrame()
{
    if (current_pass_)
    {
        SDL_EndGPURenderPass(current_pass_);
    }

    if (copy_pass_)
    {
        SDL_EndGPUCopyPass(copy_pass_);
    }

    SDL_SubmitGPUCommandBuffer(command_buffer_);
}
```
