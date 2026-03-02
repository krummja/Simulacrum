#include "renderlib/gpu_vertex_pool.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace Simulacrum {

    bool GPUVertexPool::init(SDL_GPUDevice* device, uint32_t vertex_size, size_t max_vertices) {
        if (!device) {
            spdlog::error("GPUVertexPool::init: null device");
            return false;
        }

        if (vertex_size == 0 || max_vertices == 0) {
            spdlog::error(
                "GPUVertexPool::init: invalid parameters (vertex_size={}, max_vertices={})",
                vertex_size,
                max_vertices
            );
            return false;
        }

        device_ = device;
        vertex_size_ = vertex_size;
        max_vertices_ = max_vertices;

        uint32_t buffer_size = vertex_size * static_cast<uint32_t>(max_vertices);

        // Create triple-buffered transfer buffers
        for (size_t i = 0; i < FRAME_COUNT; ++i) {
            transfer_buffers_[i] = GPUTransferBuffer(
                device,
                SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                buffer_size
            );

            if (!transfer_buffers_[i].isValid()) {
                spdlog::error("GPUVertexPool: failed to create transfer buffer {}", i);
                shutdown();
                return false;
            }
        }

        // Create persistent GPU buffer
        gpu_buffer_ = GPUBuffer(device, SDL_GPU_BUFFERUSAGE_VERTEX, buffer_size);

        if (!gpu_buffer_.isValid()) {
            spdlog::error("GPUVertexPool: failed to create GPU vertex buffer");
            shutdown();
            return false;
        }

        return true;
    }

    void GPUVertexPool::shutdown() {
        gpu_buffer_ = GPUBuffer();

        std::generate(transfer_buffers_.begin(), transfer_buffers_.end(), []() { return GPUTransferBuffer(); });

        device_ = nullptr;
        frame_index_ = 0;
        current_vertex_count_ = 0;
        mapped_ptr_ = nullptr;
    }

    GPUVertexPool::GPUVertexPool(GPUVertexPool&& other) noexcept
        : device_(other.device_)
        , transfer_buffers_(std::move(other.transfer_buffers_))
        , gpu_buffer_(std::move(other.gpu_buffer_))
        , frame_index_(other.frame_index_)
        , vertex_size_(other.vertex_size_)
        , max_vertices_(other.max_vertices_)
        , current_vertex_count_(other.current_vertex_count_)
        , pending_vertex_count_(other.pending_vertex_count_)
        , mapped_ptr_(other.mapped_ptr_)
    {
        other.device_ = nullptr;
        other.frame_index_ = 0;
        other.vertex_size_ = 0;
        other.max_vertices_ = 0;
        other.current_vertex_count_ = 0;
        other.pending_vertex_count_ = 0;
        other.mapped_ptr_ = nullptr;
    }

    GPUVertexPool& GPUVertexPool::operator=(GPUVertexPool&& other) noexcept {
        if (this != &other) {
            shutdown();

            device_ = other.device_;
            transfer_buffers_ = std::move(other.transfer_buffers_);
            gpu_buffer_ = std::move(other.gpu_buffer_);
            frame_index_ = other.frame_index_;
            vertex_size_ = other.vertex_size_;
            max_vertices_ = other.max_vertices_;
            current_vertex_count_ = other.current_vertex_count_;
            pending_vertex_count_ = other.pending_vertex_count_;
            mapped_ptr_ = other.mapped_ptr_;

            other.device_ = nullptr;
            other.frame_index_ = 0;
            other.vertex_size_ = 0;
            other.max_vertices_ = 0;
            other.current_vertex_count_ = 0;
            other.pending_vertex_count_ = 0;
            other.mapped_ptr_ = nullptr;
        }

        return *this;
    }

    void* GPUVertexPool::beginFrame() {
        if (!device_) {
            spdlog::error("GPUVertexPool::beginFrame: not initialized");
            return nullptr;
        }

        // Advance to next frame's transfer buffer
        frame_index_ = (frame_index_ + 1) % FRAME_COUNT;
        current_vertex_count_ = 0;
        pending_vertex_count_ = 0;

        // Map with cycle=true to handle if previous frame's upload is still in flight
        mapped_ptr_ = transfer_buffers_[frame_index_].map(true);

        if (!mapped_ptr_) {
            spdlog::error("GPUVertexPool::beginFrame: failed to map transfer buffer");
        }

        return mapped_ptr_;
    }

    void GPUVertexPool::endFrame(size_t vertex_count) {
        if (!device_) {
            return;
        }

        if (vertex_count > max_vertices_) {
            spdlog::warn(
                "GPUVertexPool::endFrame: vertex count {} exceeds max {}, clamping",
                vertex_count,
                max_vertices_
            );
        }

        current_vertex_count_ = vertex_count;
        transfer_buffers_[frame_index_].unmap();
        mapped_ptr_ = nullptr;
    }

    void GPUVertexPool::upload(SDL_GPUCopyPass* copy_pass) {
        if (!copy_pass || current_vertex_count_ == 0) {
            return;
        }

        SDL_GPUTransferBufferLocation src = transfer_buffers_[frame_index_].asLocation(0);

        SDL_GPUBufferRegion dst{};
        dst.buffer = gpu_buffer_.get();
        dst.offset = 0;
        dst.size = static_cast<uint32_t>(current_vertex_count_ * vertex_size_);

        SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);
    }

} // namespace Simulacrum
