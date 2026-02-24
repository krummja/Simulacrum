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

    static UniformBuffer timeUniform{};

    GPURenderer::GPURenderer(
        SDL_GPUDevice* device,
        SDL_Window* window
    )
        : device_(device)
        , window_(window)
    {}

    void GPURenderer::init() {
        spdlog::debug("    Initializing vertex shader...");
        SDL_GPUShader* vertexShader = initVertexShader();

        spdlog::debug("    Initializing fragment shader...");
        SDL_GPUShader* fragmentShader = initFragmentShader();

        // Create the graphics pipeline
        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = vertexShader;
        pipelineInfo.fragment_shader = fragmentShader;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

        // Describe the vertex buffers
        SDL_GPUVertexBufferDescription vertexBufferDescriptions[1];
        vertexBufferDescriptions[0].slot = 0;
        vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vertexBufferDescriptions[0].instance_step_rate = 0;
        vertexBufferDescriptions[0].pitch = sizeof(Vertex);

        pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
        pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescriptions;

        // Describe the vertex attribute
        SDL_GPUVertexAttribute vertexAttributes[2];

        // a_position
        vertexAttributes[0].buffer_slot = 0;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        vertexAttributes[0].offset = 0;

        // a_color
        vertexAttributes[1].buffer_slot = 0;
        vertexAttributes[1].location = 1;
        vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        vertexAttributes[1].offset = sizeof(float) * 3;

        pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
        pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

        // Describe the color target
        SDL_GPUColorTargetDescription colorTargetDescriptions[1];
        colorTargetDescriptions[0] = {};
        colorTargetDescriptions[0].blend_state.enable_blend = true;
        colorTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        colorTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(device_, window_);

        pipelineInfo.target_info.num_color_targets = 1;
        pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

        // Create the pipeline
        graphics_pipeline_ = SDL_CreateGPUGraphicsPipeline(device_, &pipelineInfo);

        // Free the shaders after setting up the pipeline
        SDL_ReleaseGPUShader(device_, vertexShader);
        SDL_ReleaseGPUShader(device_, fragmentShader);

        // Create the vertex buffer
        SDL_GPUBufferCreateInfo bufferInfo{};
        bufferInfo.size = sizeof(vertices);
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        vertex_buffer_ = SDL_CreateGPUBuffer(device_, &bufferInfo);

        // Create a transfer buffer to upload to the vertex buffer
        SDL_GPUTransferBufferCreateInfo transferInfo{};
        transferInfo.size = sizeof(vertices);
        transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_buffer_ = SDL_CreateGPUTransferBuffer(device_, &transferInfo);

        // Fill the transfer buffer
        Vertex* data = (Vertex*)SDL_MapGPUTransferBuffer(device_, transfer_buffer_, false);

        SDL_memcpy(data, (void*)vertices, sizeof(vertices));

        SDL_UnmapGPUTransferBuffer(device_, transfer_buffer_);

        // Start a copy pass
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device_);
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

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
        SDL_UploadToGPUBuffer(copyPass, &location, &region, true);

        // End the copy pass
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }

    void GPURenderer::close() {
        // Release buffers
        SDL_ReleaseGPUBuffer(device_, vertex_buffer_);
        SDL_ReleaseGPUTransferBuffer(device_, transfer_buffer_);

        // Release pipeline
        SDL_ReleaseGPUGraphicsPipeline(device_, graphics_pipeline_);
    }

    SDL_GPUShader* GPURenderer::initVertexShader() {
        size_t vertexCodeSize;

        void* vertexCode = SDL_LoadFile("assets/shaders/vertex.spv", &vertexCodeSize);

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

        SDL_GPUShader* vertexShader = SDL_CreateGPUShader(device_, &vertexInfo);

        SDL_free(vertexCode);
        return vertexShader;
    }

    SDL_GPUShader* GPURenderer::initFragmentShader() {
        size_t fragmentCodeSize;
        void* fragmentCode = SDL_LoadFile("assets/shaders/fragment.spv", &fragmentCodeSize);

        SDL_GPUShaderCreateInfo fragmentInfo{};
        fragmentInfo.code = (Uint8*)fragmentCode;
        fragmentInfo.code_size = fragmentCodeSize;
        fragmentInfo.entrypoint = "main";
        fragmentInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        fragmentInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        fragmentInfo.num_samplers = 0;
        fragmentInfo.num_storage_buffers = 0;
        fragmentInfo.num_storage_textures = 0;
        fragmentInfo.num_uniform_buffers = 0;

        SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(device_, &fragmentInfo);
        SDL_free(fragmentCode);

        return fragmentShader;
    }

    void GPURenderer::render() {
        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device_);
        SDL_GPUTexture* swapchainTexture;
        Uint32 width;
        Uint32 height;

        SDL_WaitAndAcquireGPUSwapchainTexture(
            commandBuffer,
            window_,
            &swapchainTexture,
            &width,
            &height
        );

        // End the frame early if a swapchain texture is unavailable
        if (swapchainTexture == NULL) {
            // ALWAYS submit the command buffer
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            return;
        }

        // Create the color target
        SDL_GPUColorTargetInfo colorTargetInfo{};
        colorTargetInfo.clear_color = {240/255.0f, 240/255.0f, 240/255.0f, 255/255.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = swapchainTexture;

        // Begin a render pass
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

        // Bind the pipeline
        SDL_BindGPUGraphicsPipeline(renderPass, graphics_pipeline_);

        // Bind the vertex buffer
        SDL_GPUBufferBinding bufferBindings[1];
        bufferBindings[0].buffer = vertex_buffer_;
        bufferBindings[0].offset = 0;

        SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, 1);

        // Update the time uniform
        timeUniform.time = SDL_GetTicksNS() / 1e9f;
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &timeUniform, sizeof(UniformBuffer));

        // Issue a draw call
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

        // End the render pass
        SDL_EndGPURenderPass(renderPass);

        // Submit the command buffer
        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }
}
