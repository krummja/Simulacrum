#include "renderlib/gpu_sampler.hpp"

namespace Simulacrum {

    GPUSampler::GPUSampler(
        SDL_GPUDevice* device,
        SDL_GPUFilter min_mag_filter,
        SDL_GPUSamplerAddressMode address_mode
    ) {}

    GPUSampler::GPUSampler(
        SDL_GPUDevice* device,
        const SDL_GPUSamplerCreateInfo& create_info
    ) {

    }

    GPUSampler::~GPUSampler() {
        release();
    }

    GPUSampler::GPUSampler(GPUSampler&& other) noexcept
        : sampler_(other.sampler_)
        , device_(other.device_)
    {
        other.sampler_ = nullptr;
        other.device_ = nullptr;
    }

    GPUSampler& GPUSampler::operator=(GPUSampler&& other) noexcept {
        if (this != &other) {
            release();

            sampler_ = other.sampler_;
            device_ = other.device_;

            other.sampler_ = nullptr;
            other.device_ = nullptr;
        }
        return *this;
    }

    GPUSampler GPUSampler::createNearest(SDL_GPUDevice* device) {
        return GPUSampler(
            device,
            SDL_GPU_FILTER_NEAREST,
            SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        );
    }

    GPUSampler GPUSampler::createLinear(SDL_GPUDevice* device) {
        return GPUSampler(
            device,
            SDL_GPU_FILTER_LINEAR,
            SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        );
    }

    GPUSampler GPUSampler::createLinearMipmapped(SDL_GPUDevice* device) {
        if (!device) {
            return GPUSampler();
        }

        SDL_GPUSamplerCreateInfo create_info{};
        create_info.min_filter = SDL_GPU_FILTER_LINEAR;
        create_info.mag_filter = SDL_GPU_FILTER_LINEAR;
        create_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
        create_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        create_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        create_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        create_info.mip_lod_bias = 0.0f;
        create_info.max_anisotropy = 1.0f;
        create_info.compare_op = SDL_GPU_COMPAREOP_NEVER;
        create_info.min_lod = 0.0f;
        create_info.max_lod = 1000.0f;
        create_info.enable_anisotropy = false;
        create_info.enable_compare = false;

        return GPUSampler(device, create_info);
    }

    void GPUSampler::release() {
        if (sampler_ && device_) {
            SDL_ReleaseGPUSampler(device_, sampler_);
            sampler_ = nullptr;
        }
    }

} // namespace Simulacrum
