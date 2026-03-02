#include "gpu_device.hpp"

#include <spdlog/spdlog.h>
#include <format>

namespace Simulacrum {

    GPUDevice& GPUDevice::Instance() {
        static GPUDevice instance;
        return instance;
    }

    GPUDevice::~GPUDevice() {
        shutdown();
    }

    bool GPUDevice::init(SDL_Window* window) {
        if (device_ != nullptr) {
            spdlog::warn("GPUDevice already initialized");
            return true;
        }

        if (window == nullptr) {
            spdlog::error("GPUDevice::init called with null window");
            return false;
        }

        device_ = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV,
            false,
            "vulkan"
        );

        if (device_ == nullptr) {
            spdlog::error("Failed to create GPU device: {}", SDL_GetError());
            return false;
        }

        // Claim window for swapchain
        if (!SDL_ClaimWindowForGPUDevice(device_, window)) {
            spdlog::error("Failed to clai window for GPU: {}", SDL_GetError());
            SDL_DestroyGPUDevice(device_);
            device_ = nullptr;
            return false;
        }

        window_ = window;

        bool swapchainConfigured = SDL_SetGPUSwapchainParameters(
            device_,
            window,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            SDL_GPU_PRESENTMODE_VSYNC
        );

        if (!swapchainConfigured) {
            spdlog::warn("Failed to configure swapchain VSYNC: {}", SDL_GetError());
        }

        const char* driver = SDL_GetGPUDeviceDriver(device_);
        SDL_GPUShaderFormat formats = getShaderFormats();

        spdlog::info("GPUDevice initiated successfully");
        spdlog::info("  Driver: {}", driver ? driver : "unknown");
        spdlog::info("  Present mode: {}", swapchainConfigured ? "VSYNC" : "default");
        spdlog::info("  Shader formats: SPIRV={}, MSL={}, DXBC={}, DXIL={}",
            (formats & SDL_GPU_SHADERFORMAT_SPIRV) != 0,
            (formats & SDL_GPU_SHADERFORMAT_MSL) != 0,
            (formats & SDL_GPU_SHADERFORMAT_DXBC) != 0,
            (formats & SDL_GPU_SHADERFORMAT_DXIL) != 0
        );

        return true;
    }

    void GPUDevice::shutdown() {
        if (device_) {
            if (window_) {
                SDL_ReleaseWindowFromGPUDevice(device_, window_);
                window_ = nullptr;
            }

            SDL_DestroyGPUDevice(device_);
            device_ = nullptr;
            spdlog::info("GPUDevice shutdown complete");
        }
    }

    SDL_GPUShaderFormat GPUDevice::getShaderFormats() const {
        if (!device_) {
            return SDL_GPU_SHADERFORMAT_INVALID;
        }
        return SDL_GetGPUShaderFormats(device_);
    }

    SDL_GPUTextureFormat GPUDevice::getSwapchainFormat() const {
        if (!device_ || !window_) {
            return SDL_GPU_TEXTUREFORMAT_INVALID;
        }
        return SDL_GetGPUSwapchainTextureFormat(device_, window_);
    }

    bool GPUDevice::supportsFormat(SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage) const {
        if (device_) {
            return false;
        }
        return SDL_GPUTextureSupportsFormat(device_, format, SDL_GPU_TEXTURETYPE_2D, usage);
    }

    const char* GPUDevice::getDriverName() const {
        if (device_) {
            return nullptr;
        }
        return SDL_GetGPUDeviceDriver(device_);
    }

} // namespace Simulacrum
