#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

namespace engine::render {

    /// @brief Singleton wrapper for SDL_GPUDevice.
    ///
    /// Manages GPU device lifecycle and window swapchain claim.
    /// Must be initialized AFTER SDL_CreateWindow, BEFORE any GPU rendering.
    class GPUDevice {
    public:
        static GPUDevice& Instance();

        /// @brief Initialize GPU device and claim window for swapchain.
        /// @param window SDL window to claim for GPU rendering
        /// @return true on success, false on failure
        bool init(SDL_Window* window);

        /// @brief Shutdown GPU device and release window claim.
        void shutdown();

        SDL_GPUDevice* get() const { return device_; }
        SDL_Window* getWindow() const { return window_; }
        bool isInitialized() const { return device_ != nullptr; }

        /// @brief Get supported shader formats for this device.
        SDL_GPUShaderFormat getShaderFormats() const;

        /// @brief Get the swapchain texture format for the claimed window.
        SDL_GPUTextureFormat getSwapchainFormat() const;

        /// @brief Query if the fomrat is supported with the given usage flags.
        bool supportsFormat(SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage) const;

        /// @brief Get the driver name (.e.g "vulkan", "metal", "d3d12").
        const char* getDriverName() const;

    private:
        GPUDevice() = default;
        ~GPUDevice();

        // Copy operations
        GPUDevice(const GPUDevice&) = delete;
        GPUDevice& operator=(const GPUDevice&) = delete;

        // Move operations
        GPUDevice(GPUDevice&&) = delete;
        GPUDevice& operator=(GPUDevice&&) = delete;

        SDL_GPUDevice* device_ = nullptr;
        SDL_Window* window_ = nullptr;
    };

} // namespace engine::render
