#pragma once

#include <spdlog/spdlog.h>
#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include "TimestepManager.hpp"
#include "GpuDevice.hpp"
#include "GpuBuffer.hpp"
#include "GpuTexture.hpp"
#include "GpuTransferBuffer.hpp"
#include "GpuSampler.hpp"
#include "GpuTypes.hpp"


class GPUTestFixture
{
public:
    GPUTestFixture()
    {
        if (!sdl_initialized_)
        {
            if (!SDL_Init(SDL_INIT_VIDEO))
            {
                spdlog::error("SDL video initialization failed: {}", SDL_GetError());
                gpu_available_ = false;
            }
            else
            {
                sdl_initialized_ = true;
                checkGPUAvailability();
            }
        }
    }

    virtual ~GPUTestFixture() {}

    static bool isGPUAvailable()
    {
        return gpu_available_;
    }

    static SDL_Window* getTestWindow()
    {
        if (!test_window_ && sdl_initialized_)
        {
            test_window_ = SDL_CreateWindow(
                "GPU Test Window",
                64, 65,
                SDL_WINDOW_HIDDEN
            );

            if (!test_window_)
            {
                spdlog::error("Failed to create test window: {}", SDL_GetError());
            }
        }

        return test_window_;
    }

    static void showTestWindow()
    {
        if (test_window_)
        {
            SDL_ShowWindow(test_window_);
            SDL_Event evt;
            while (SDL_PollEvent(&evt)) {}
        }
    }

    static void hideTestWindow()
    {
        if (test_window_)
        {
            SDL_HideWindow(test_window_);
        }
    }

    static void cleanup()
    {
        if (test_window_)
        {
            SDL_DestroyWindow(test_window_);
            test_window_ = nullptr;
        }

        if (sdl_initialized_)
        {
            SDL_Quit();
            sdl_initialized_ = false;
        }

        gpu_available_ = false;
    }

protected:
    static void checkGPUAvailability()
    {
        SDL_Window* test_win = SDL_CreateWindow(
            "GPU Test",
            64, 64,
            SDL_WINDOW_HIDDEN
        );

        if (!test_win)
        {
            spdlog::error("Cannot create window for GPU Test: {}", SDL_GetError());
            gpu_available_ = false;
            return;
        }

        SDL_GPUDevice* device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV,
            false,   // debug mode
            nullptr  // name
        );

        if (!device)
        {
            spdlog::error("Cannot create GPU device: {}", SDL_GetError());
            SDL_DestroyWindow(test_win);
            gpu_available_ = false;
            return;
        }

        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(test_win);

        gpu_available_ = true;
        spdlog::info("GPU is available for testing");
    }

    static inline bool sdl_initialized_ = false;
    static inline bool gpu_available_ = false;
    static inline SDL_Window* test_window_ = nullptr;

private:
    static int uniqueID;
};


struct GPUGlobalFixture
{
    ~GPUGlobalFixture()
    {
        GPUTestFixture::cleanup();
    }
};
