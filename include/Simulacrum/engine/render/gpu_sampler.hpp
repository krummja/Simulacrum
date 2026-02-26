#pragma once

#include <SDL3/SDL_gpu.h>

namespace engine::render {

    /// @brief RAII wrapper for SDL_GPUSampler.
    /// Provides preset samplers (nearest, linear) and custom sampler creation.
    class GPUSampler {
    public:
        GPUSampler() = default;

        /// @brief Create sampler with specified filter modes.
        /// @param device GPU device
        /// @param min_mag_filter Filter mode for min/mag filtering
        /// @param address_mode address mode for UV coordinates
        GPUSampler(
            SDL_GPUDevice* device,
            SDL_GPUFilter min_mag_filter,
            SDL_GPUSamplerAddressMode address_mode = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        );

        /// @brief Create sampler with full control over all parameters.
        GPUSampler(SDL_GPUDevice* device, const SDL_GPUSamplerCreateInfo& create_info);

        ~GPUSampler();

        // Move-only
        GPUSampler(GPUSampler&&) noexcept;
        GPUSampler& operator=(GPUSampler&&) noexcept;
        GPUSampler(const GPUSampler&) = delete;
        GPUSampler& operator=(const GPUSampler&) = delete;

        SDL_GPUSampler* get() const { return sampler_; }
        bool isValid() const { return sampler_ != nullptr; }

        /// @brief Create a nearest-neighbor sampler (pixel-perfect for 2D).
        static GPUSampler createNearest(SDL_GPUDevice* device);

        /// @brief Create a linear filtering sampler (smooth for zoom).
        static GPUSampler createLinear(SDL_GPUDevice* device);

        /// @brief Create a linear sampler with mipmapping.
        static GPUSampler createLinearMipmapped(SDL_GPUDevice* device);

    private:
        void release();

        SDL_GPUSampler* sampler_{nullptr};
        SDL_GPUDevice* device_{nullptr};
    };

} // namespace engine::render
