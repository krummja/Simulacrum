#include "gpu_buffer.hpp"

#include <spdlog/spdlog.h>
#include <format>

namespace Simulacrum {

    GPUBuffer::GPUBuffer(SDL_GPUDevice* device, SDL_GPUBufferUsageFlags usage, uint32_t size)
        : device_(device)
        , size_(size)
        , usage_(usage)
    {
        if (!device) {
            spdlog::error("GPUBuffer: null device");
            return;
        }

        if (size == 0) {
            spdlog::error("GPUBuffer: invalid size 0");
            return;
        }

        SDL_GPUBufferCreateInfo create_info{};
        create_info.usage = usage;
        create_info.size = size;

        buffer_ = SDL_CreateGPUBuffer(device, &create_info);

        if (!buffer_) {
            spdlog::critical("Failed to create GPU buffer ({} bytes): {}", size, SDL_GetError());
        }
    }

    GPUBuffer::~GPUBuffer() {
        release();
    }

    GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept
        : buffer_(other.buffer_)
        , device_(other.device_)
        , size_(other.size_)
        , usage_(other.usage_)
    {
        other.buffer_ = nullptr;
        other.device_ = nullptr;
        other.size_ = 0;
    }

    GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other) noexcept {
        if (this != &other) {
            release();

            buffer_ = other.buffer_;
            device_ = other.device_;
            size_ = other.size_;
            usage_ = other.usage_;

            other.buffer_ = nullptr;
            other.device_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void GPUBuffer::release() {
        if (buffer_ && device_) {
            SDL_ReleaseGPUBuffer(device_, buffer_);
            buffer_ = nullptr;
        }
    }

    SDL_GPUBufferBinding GPUBuffer::asBinding(uint32_t offset) const {
        if (!buffer_) {
            spdlog::warn("GPUBuffer::asBinding() called on invalid buffer");
        }

        SDL_GPUBufferBinding binding{};
        binding.buffer = buffer_;
        binding.offset = offset;
        return binding;
    }

    SDL_GPUBufferRegion GPUBuffer::asRegion(uint32_t offset, uint32_t size) const {
        if (!buffer_) {
            spdlog::warn("GPUBuffer::asRegion() called on invalid buffer");
        }
        if (offset > size_) {
            spdlog::warn("GPUBuffer::asRegion() offset {} exceeds buffer size {}", offset, size_);
            offset = size_;
        }

        SDL_GPUBufferRegion region{};
        region.buffer = buffer_;
        region.offset = offset;
        region.size = (size == 0) ? (size_ - offset) : size;
        return region;
    }

} // namespace Simulacrum
