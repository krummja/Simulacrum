#include "gpu_renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace engine::render {

    static Vertex vertices[] {
        {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top vertex
        {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
        {0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}     // bottom right vertex
    };

    static UniformBuffer time_uniform{};

    GPURenderer::GPURenderer(
        SDL_GPUDevice* device,
        SDL_Window* window
    )
        : device_(device)
        , window_(window)
    {}


    /// PUBLIC API

    void GPURenderer::close() {
        // Release buffers
        SDL_ReleaseGPUBuffer(device_, vertex_buffer_);
        SDL_ReleaseGPUTransferBuffer(device_, transfer_buffer_);

        // Release pipeline
        SDL_ReleaseGPUGraphicsPipeline(device_, graphics_pipeline_);
    }

    void GPURenderer::init() {
        draw_queue_ = std::make_unique<DrawQueue>();
        draw_queue_->init();

        // Create the graphics pipeline
        initGraphicsPipeline();

        // Create the vertex buffer
        initVertexBuffer();

        // Create a transfer buffer to upload to the vertex buffer
        initTransferBuffer();

        // Fill the transfer buffer
        fillTransferBuffer();

        // Do the copy pass to move from CPU to GPU
        executeCopyPass();
    }

    void GPURenderer::submit(MeshGPU* mesh) {
        draw_queue_->addMesh(mesh);
    }

    void GPURenderer::render() {

        // Acquire the command buffer
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device_);

        // Acquire the swapchain texture from the GPU
        SDL_GPUTexture* swapchain_texture;
        Uint32 width;
        Uint32 height;

        SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer,
            window_,
            &swapchain_texture,
            &width,
            &height
        );

        // End the frame early if a swapchain texture is unavailable
        if (swapchain_texture == NULL) {
            // ALWAYS submit the command buffer
            SDL_SubmitGPUCommandBuffer(command_buffer);
            return;
        }

        // Create the color target
        SDL_GPUColorTargetInfo color_target_info = createColorTarget(swapchain_texture);

        // Begin a render pass
        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, NULL);

        // Bind the pipeline
        SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline_);

        // Bind the vertex buffer
        SDL_GPUBufferBinding buffer_bindings[1];
        buffer_bindings[0].buffer = vertex_buffer_;
        buffer_bindings[0].offset = 0;
        SDL_BindGPUVertexBuffers(render_pass, 0, buffer_bindings, 1);

        // Update the time uniform
        time_uniform.time = SDL_GetTicksNS() / 1e9f;
        SDL_PushGPUFragmentUniformData(command_buffer, 0, &time_uniform, sizeof(UniformBuffer));

        // Issue a draw call
        SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

        // End the render pass
        SDL_EndGPURenderPass(render_pass);

        // Submit the command buffer
        SDL_SubmitGPUCommandBuffer(command_buffer);
    }


    /// PRIVATE API

    SDL_GPUColorTargetInfo GPURenderer::createColorTarget(SDL_GPUTexture* swapchain_texture) {
        SDL_GPUColorTargetInfo color_target_info{};
        color_target_info.clear_color = {240/255.0f, 240/255.0f, 240/255.0f, 255/255.0f};
        color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target_info.store_op = SDL_GPU_STOREOP_STORE;
        color_target_info.texture = swapchain_texture;
        return color_target_info;
    }

    SDL_GPUShader* GPURenderer::initVertexShader() {
        // Load the vertex shader from file
        size_t vertexCodeSize;
        void* vertexCode = SDL_LoadFile(
            "assets/shaders/vertex.spv",
            &vertexCodeSize
        );

        // Build out shader info
        SDL_GPUShaderCreateInfo vertexInfo{};
        vertexInfo.code = (Uint8*)vertexCode;
        vertexInfo.code_size = vertexCodeSize;
        vertexInfo.entrypoint = "main";
        vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
        vertexInfo.num_samplers = 0;
        vertexInfo.num_storage_buffers = 0;
        vertexInfo.num_storage_textures = 0;
        vertexInfo.num_uniform_buffers = 0;

        // Build the shader on the device
        SDL_GPUShader* vertex_shader = SDL_CreateGPUShader(device_, &vertexInfo);

        // Free the loaded shader code
        SDL_free(vertexCode);
        return vertex_shader;
    }

    SDL_GPUShader* GPURenderer::initFragmentShader() {
        size_t fragment_code_size;
        void* fragment_code = SDL_LoadFile(
            "assets/shaders/fragment.spv",
            &fragment_code_size
        );

        SDL_GPUShaderCreateInfo fragment_info{};
        fragment_info.code = (Uint8*)fragment_code;
        fragment_info.code_size = fragment_code_size;
        fragment_info.entrypoint = "main";
        fragment_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
        fragment_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        fragment_info.num_samplers = 0;
        fragment_info.num_storage_buffers = 0;
        fragment_info.num_storage_textures = 0;
        fragment_info.num_uniform_buffers = 0;

        SDL_GPUShader* fragment_shader = SDL_CreateGPUShader(device_, &fragment_info);
        SDL_free(fragment_code);

        return fragment_shader;
    }

    void GPURenderer::initGraphicsPipeline() {
        // Initialize shaders
        SDL_GPUShader* vertex_shader = initVertexShader();
        SDL_GPUShader* fragment_shader = initFragmentShader();

        // Create the graphics pipeline
        SDL_GPUGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.vertex_shader = vertex_shader;
        pipeline_info.fragment_shader = fragment_shader;
        pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

        // Describe the vertex buffers
        SDL_GPUVertexBufferDescription vertex_buffer_descriptions[1];
        vertex_buffer_descriptions[0].slot = 0;
        vertex_buffer_descriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vertex_buffer_descriptions[0].instance_step_rate = 0;
        vertex_buffer_descriptions[0].pitch = sizeof(Vertex);

        pipeline_info.vertex_input_state.num_vertex_buffers = 1;
        pipeline_info.vertex_input_state.vertex_buffer_descriptions = vertex_buffer_descriptions;

        // Describe the vertex attribute
        SDL_GPUVertexAttribute vertex_attributes[2];

        // a_position
        vertex_attributes[0].buffer_slot = 0;
        vertex_attributes[0].location = 0;
        vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        vertex_attributes[0].offset = 0;

        // a_color
        vertex_attributes[1].buffer_slot = 0;
        vertex_attributes[1].location = 1;
        vertex_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        vertex_attributes[1].offset = sizeof(float) * 3;

        pipeline_info.vertex_input_state.num_vertex_attributes = 2;
        pipeline_info.vertex_input_state.vertex_attributes = vertex_attributes;

        // Describe the color target
        SDL_GPUColorTargetDescription color_target_descriptions[1];
        color_target_descriptions[0] = {};
        color_target_descriptions[0].blend_state.enable_blend = true;
        color_target_descriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        color_target_descriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        color_target_descriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        color_target_descriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        color_target_descriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        color_target_descriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        color_target_descriptions[0].format = SDL_GetGPUSwapchainTextureFormat(device_, window_);

        pipeline_info.target_info.num_color_targets = 1;
        pipeline_info.target_info.color_target_descriptions = color_target_descriptions;

        // Create the pipeline
        graphics_pipeline_ = SDL_CreateGPUGraphicsPipeline(device_, &pipeline_info);

        // Free the shaders after setting up the pipeline
        SDL_ReleaseGPUShader(device_, vertex_shader);
        SDL_ReleaseGPUShader(device_, fragment_shader);
    }

    void GPURenderer::initVertexBuffer() {
        SDL_GPUBufferCreateInfo buffer_info{};
        buffer_info.size = sizeof(vertices);
        buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        vertex_buffer_ = SDL_CreateGPUBuffer(device_, &buffer_info);
    }

    void GPURenderer::initTransferBuffer() {
        SDL_GPUTransferBufferCreateInfo transfer_info{};
        transfer_info.size = sizeof(vertices);
        transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_buffer_ = SDL_CreateGPUTransferBuffer(device_, &transfer_info);
    }

    void GPURenderer::fillTransferBuffer() {
        Vertex* data = (Vertex*)SDL_MapGPUTransferBuffer(device_, transfer_buffer_, false);
        SDL_memcpy(data, (void*)vertices, sizeof(vertices));
        SDL_UnmapGPUTransferBuffer(device_, transfer_buffer_);
    }

    void GPURenderer::executeCopyPass() {
        // Start a copy pass
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device_);
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

        // Where is the data
        SDL_GPUTransferBufferLocation location{};
        location.transfer_buffer = transfer_buffer_;
        location.offset = 0;

        // Where to upload the data
        SDL_GPUBufferRegion region{};
        region.buffer = vertex_buffer_;
        region.size = sizeof(vertices);
        region.offset = 0;

        // Upload the data
        SDL_UploadToGPUBuffer(copy_pass, &location, &region, true);

        // End the copy pass
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(command_buffer);
    }

} // namespace engine::render
