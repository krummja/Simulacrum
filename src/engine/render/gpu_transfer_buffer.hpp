#pragma once

#include <SDL3/SDL_gpu.h>
#include <cstdint>

namespace engine::render {

    /// @brief RAII wrapper for SDL_GPUTransferBuffer.
    /// Used in CPU-to-GPU data trasfers (uploads) and GPU-to-CPU reads.
    /// Supports mapping/unmapping for direct CPU access.
    class GPUTransferBuffer {
    public:
        GPUTransferBuffer() = default;

        /// @brief Create a transfer buffer.
        /// @param device GPU device
        /// @param usage UPLOAD for CPU->GPU, DOWNLOAD for GPU->CPU
        /// @param size Buffer size in bytes
        GPUTransferBuffer(
            SDL_GPUDevice* device,
            SDL_GPUTransferBufferUsage usage,
            uint32_t size
        );

        ~GPUTransferBuffer();

        // Move-only
        GPUTransferBuffer(GPUTransferBuffer&& other) noexcept;
        GPUTransferBuffer& operator=(GPUTransferBuffer&& other) noexcept;
        GPUTransferBuffer(const GPUTransferBuffer&) = delete;
        GPUTransferBuffer& operator=(const GPUTransferBuffer&) = delete;

        SDL_GPUTransferBuffer* get() const { return buffer_; }
        uint32_t getSize() const { return size_; }
        bool isValid() const { return buffer_ != nullptr; }
        bool isMapped() const { return mapped_; }

        /// @brief Map the buffer for CPU access.
        /// @param cycle If true, allows reusing the buffer before previous operations
        /// complete.
        /// @return Pointer to mapped memory, or nullptr on failure
        void* map(bool cycle = true);

        /// @brief Unmap the buffer. Must be called before using in copy operations.
        void unmap();

        /// @brief Create transfer buffer location for copy operations.
        /// @param offset Byte offset into the buffer
        SDL_GPUTransferBufferLocation asLocation(uint32_t offset = 0) const;

    private:
        void release();

        SDL_GPUTransferBuffer* buffer_{nullptr};
        SDL_GPUDevice* device_{nullptr};
        uint32_t size_{0};
        bool mapped_{false};

    };

} // namespace engine::render
