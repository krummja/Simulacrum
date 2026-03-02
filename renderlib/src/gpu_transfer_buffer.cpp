#include "renderlib/gpu_transfer_buffer.hpp"
#include <spdlog/spdlog.h>

namespace Simulacrum {

    GPUTransferBuffer::GPUTransferBuffer(
        SDL_GPUDevice* device,
        SDL_GPUTransferBufferUsage usage,
        uint32_t size
    )
        : device_(device)
        , size_(size)
    {
        if (!device) {
            spdlog::error("GPUTransferBuffer: null device");
            return;
        }

        if (size == 0) {
            spdlog::error("GPUTransferBuffer: invalid size 0");
            return;
        }

        SDL_GPUTransferBufferCreateInfo create_info{};
        create_info.usage = usage;
        create_info.size = size;

        buffer_ = SDL_CreateGPUTransferBuffer(device, &create_info);

        if (!buffer_) {
            spdlog::error("Failed to create GPU transfer buffer ({} bytes): {}", size, SDL_GetError());
        }
    }

    GPUTransferBuffer::~GPUTransferBuffer() {
        release();
    }

    GPUTransferBuffer::GPUTransferBuffer(GPUTransferBuffer&& other) noexcept
        : buffer_(other.buffer_)
        , device_(other.device_)
        , size_(other.size_)
        , mapped_(other.mapped_)
    {
        other.buffer_ = nullptr;
        other.device_ = nullptr;
        other.size_ = 0;
        other.mapped_ = false;
    }

    GPUTransferBuffer& GPUTransferBuffer::operator=(GPUTransferBuffer&& other) noexcept {
        if (this != &other) {
            release();

            buffer_ = other.buffer_;
            device_ = other.device_;
            size_ = other.size_;
            mapped_ = other.mapped_;

            other.buffer_ = nullptr;
            other.device_ = nullptr;
            other.size_ = 0;
            other.mapped_ = false;
        }

        return *this;
    }

    void GPUTransferBuffer::release() {
        if (buffer_ && device_) {
            if (mapped_) {
                unmap();
            }

            SDL_ReleaseGPUTransferBuffer(device_, buffer_);
            buffer_ = nullptr;
        }
    }

    void* GPUTransferBuffer::map(bool cycle) {
        if (!buffer_ || !device_) {
            spdlog::error("GPUTransferBuffer::map: invalid buffer");
            return nullptr;
        }

        if (mapped_) {
            spdlog::warn("GPUTransferBuffer::map: already mapped");
            return nullptr;
        }

        void* ptr = SDL_MapGPUTransferBuffer(device_, buffer_, cycle);

        if (!ptr) {
            spdlog::critical("Failed to map GPU transfer buffer: {}", SDL_GetError());
            return nullptr;
        }

        mapped_ = true;
        return ptr;
    }

    void GPUTransferBuffer::unmap() {
        if (!buffer_ || !device_) {
            return;
        }

        if (!mapped_) {
            return;
        }

        SDL_UnmapGPUTransferBuffer(device_, buffer_);
        mapped_ = false;
    }

    SDL_GPUTransferBufferLocation GPUTransferBuffer::asLocation(uint32_t offset) const {
        SDL_GPUTransferBufferLocation location{};
        location.transfer_buffer = buffer_;
        location.offset = offset;
        return location;
    }

} // namespace Simulacrum
