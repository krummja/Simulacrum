#pragma once

#include <SDL3/SDL_gpu.h>
#include <cstdint>

namespace Simulacrum {

    /// @brief RAII wrapper for SDL_GPUBuffer.
    ///
    /// Used for vertex buffers, index buffers and uniform buffers on the GPU.
    /// Data must be uploaded to the transfer buffer and copy passes.
    class GPUBuffer {
    public:
        GPUBuffer() = default;

        /// @brief Create a GPU buffer with specified usage and size.
        /// @param device GPU device
        /// @param usage Buffer usage flags (VERTEX, INDEX, etc.)
        /// @param size Buffer size in bytes
        GPUBuffer(SDL_GPUDevice* device, SDL_GPUBufferUsageFlags usage, uint32_t size);

        ~GPUBuffer();

        // Move only
        GPUBuffer(GPUBuffer&&) noexcept;
        GPUBuffer& operator=(GPUBuffer&&) noexcept;
        GPUBuffer(const GPUBuffer&) = delete;
        GPUBuffer& operator=(const GPUBuffer&) = delete;

        SDL_GPUBuffer* get() const { return buffer_; }
        uint32_t getSize() const { return size_; }
        SDL_GPUBufferUsageFlags getUsage() const { return usage_; }
        bool isValid() const { return buffer_ != nullptr; }

        /// @brief Create a buffer binding for use in draw calls.
        /// @param offset Byte offset into the buffer
        SDL_GPUBufferBinding asBinding(uint32_t offset = 0) const;

        /// @brief Create a buffer region for copy operations.
        /// @param offset Byte offset into the buffer
        /// @param size  Size of the region (0  = entire buffer from offset)
        SDL_GPUBufferRegion asRegion(uint32_t offset = 0, uint32_t size = 0) const;

    private:
        void release();

        SDL_GPUBuffer* buffer_ = nullptr;
        SDL_GPUDevice* device_ = nullptr;
        uint32_t size_ = 0;
        SDL_GPUBufferUsageFlags usage_ = 0;
    };

} // namespace Simulacrum
