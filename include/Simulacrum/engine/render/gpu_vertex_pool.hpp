#pragma once

#include "gpu_buffer.hpp"
#include "gpu_transfer_buffer.hpp"
#include <array>
#include <cstdint>

namespace engine::render {

    /// @brief Triple-buffered vertex pool for zero-allocation per-frame rendering.
    ///
    /// Uses cycling transfer buffers to avoid GPU stalls:
    /// - Frame N: GPU reads from buffer 0
    /// - Frame N+1: GPU reads from buffer 1, CPU writes to buffer 0
    /// - Frame N+2: GPU reads from buffer 2, CPU writes to buffer 1
    ///
    /// This allows CPU vertex generation to overlap with CPU rendering.
    class GPUVertexPool {
    public:
        static constexpr size_t FRAME_COUNT = 3;
        static constexpr size_t DEFAULT_VERTEX_CAPACITY = 150000;  // 4K + zoom headroom

        GPUVertexPool() = default;
        ~GPUVertexPool() = default;

        // Move-only (explicit to properly clear source state)
        GPUVertexPool(GPUVertexPool&&) noexcept;
        GPUVertexPool& operator=(GPUVertexPool&&) noexcept;
        GPUVertexPool(const GPUVertexPool&) = delete;
        GPUVertexPool& operator=(const GPUVertexPool&) = delete;

        /// @brief Initializes the vertex pool.
        /// @param device GPU device
        /// @param vertex_size Size of a single vertex in bytes
        /// @param max_vertices Maximum number of vertices to support
        /// @return true on success
        bool init(SDL_GPUDevice* device, uint32_t vertex_size, size_t max_vertices = DEFAULT_VERTEX_CAPACITY);

        /// @brief Shutdown and release all buffers
        void shutdown();

        /// @brief Begin a new frame. Advances the frame index and maps the transfer buffer.
        /// @return Pointer to mapped memory for writing vertices
        void* beginFrame();

        /// @brief End the current frame. Unmaps the buffer and records vertex count.
        /// @param vertex_count Number of vertices written this frame
        void endFrame(size_t vertex_count);

        /// @brief Upload vertex data to the GPU buffer.
        /// Must be called during a copy pass.
        /// @param copy_pass Active copy pass
        void upload(SDL_GPUCopyPass* copy_pass);

        SDL_GPUBuffer* getGPUBuffer() const { return gpu_buffer_.get(); }
        size_t getVertexCount() const { return current_vertex_count_; }
        size_t getMaxVertices() const { return max_vertices_; }
        uint32_t getVertexSize() const { return vertex_size_; }
        bool isInitialized() const { return device_ != nullptr; }

        /// @brief Get the currently mapped pointer for vertex writes.
        /// Only valid between `beginFrame()` and `endFrame()`.
        /// @return Mapped memory pointer, or nullptr if not mapped
        void* getMappedPtr() const { return mapped_ptr_; }

        /// @brief Set the vertex count for manual vertex writing.
        /// Call this if writing vertices directly to `getMappedPtr()` instead of using `SpriteBatch`.
        /// @param count Number of vertices written
        void setWrittenVertexCount(size_t count) { pending_vertex_count_ = count; }

        /// @brief Get the pending vertex count set by `setWrittenVertexCount()`.
        /// @return Pending vertex count, or 0 if not set
        size_t getPendingVertexCount() const { return pending_vertex_count_; }

    private:
        SDL_GPUDevice* device_{nullptr};

        // CPU-side staging (triple-buffered)
        std::array<GPUTransferBuffer, FRAME_COUNT> transfer_buffers_;

        // GPU-side persistent buffer
        GPUBuffer gpu_buffer_;

        uint32_t frame_index_{0};
        uint32_t vertex_size_{0};
        size_t max_vertices_{0};
        size_t current_vertex_count_{0};
        size_t pending_vertex_count_{0};
        void* mapped_ptr_{nullptr};
    };

} // namespace engine::render
