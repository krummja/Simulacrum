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
#include "SpriteBatch.hpp"

#include "GpuTestFixture.hpp"


struct ResourceTestFixture : public GPUTestFixture
{
    ResourceTestFixture()
    {
        if (!isGPUAvailable()) return;

        device = &Simulacrum::GPUDevice::Instance();
        if (device->isInitialized())
        {
            device->shutdown();
        }

        SDL_Window* window = getTestWindow();
        if (window)
        {
            device->init(window);
        }
    }

    ~ResourceTestFixture()
    {
        if (device && device->isInitialized())
        {
            device->shutdown();
        }
    }

    Simulacrum::GPUDevice* device = nullptr;
};


TEST_CASE_PERSISTENT_FIXTURE(ResourceTestFixture, "Resource Tests")
{
    SECTION("Create Vertex Buffer")
    {
        REQUIRE(device->isInitialized());

        const uint32_t buffer_size = 1024;
        Simulacrum::GPUBuffer buffer(device->get(), SDL_GPU_BUFFERUSAGE_VERTEX, buffer_size);

        REQUIRE(buffer.isValid());
        REQUIRE(buffer.get() != nullptr);
        REQUIRE(buffer.getSize() == buffer_size);
        REQUIRE(buffer.getUsage() == SDL_GPU_BUFFERUSAGE_VERTEX);
    }

    SECTION("Create Index Buffer")
    {
        REQUIRE(device->isInitialized());

        const uint32_t buffer_size = 512;
        Simulacrum::GPUBuffer buffer(device->get(), SDL_GPU_BUFFERUSAGE_INDEX, buffer_size);

        REQUIRE(buffer.isValid());
        REQUIRE(buffer.getSize() == buffer_size);
        REQUIRE(buffer.getUsage() == SDL_GPU_BUFFERUSAGE_INDEX);
    }

    SECTION("Create Sampler Texture")
    {
        REQUIRE(device->isInitialized());

        Simulacrum::GPUTexture texture(
            device->get(),
            256, 256,
            SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            SDL_GPU_TEXTUREUSAGE_SAMPLER
        );

        REQUIRE(texture.isValid());
        REQUIRE(texture.get() != nullptr);
        REQUIRE(texture.getWidth() == 256);
        REQUIRE(texture.getHeight() == 256);
        REQUIRE(texture.getFormat() == SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM);
        REQUIRE(texture.isSampler());
        REQUIRE(!texture.isRenderTarget());
    }
}

TEST_CASE_PERSISTENT_FIXTURE(ResourceTestFixture, "Sprite Batch Tests")
{
    SECTION("Begin Sets State")
    {
        REQUIRE(device->isInitialized());

        Simulacrum::SpriteBatch batch;
        batch.init(device->get());

        std::vector<Simulacrum::SpriteVertex> vertices(Simulacrum::SpriteBatch::MAX_VERTICES);

        batch.begin(vertices.data(), vertices.size(), nullptr, nullptr, 256.0f, 256.0f);

        REQUIRE(batch.getSpriteCount() == 0u);
        REQUIRE(!batch.hasSprites());

        batch.end();
        batch.shutdown();
    }

    SECTION("Draw Increments Sprite Count")
    {
        REQUIRE(device->isInitialized());

        Simulacrum::SpriteBatch batch;
        batch.init(device->get());

        std::vector<Simulacrum::SpriteVertex> vertices(Simulacrum::SpriteBatch::MAX_VERTICES);
        batch.begin(vertices.data(), vertices.size(), nullptr, nullptr, 256.0f, 256.0f);

        batch.draw(0, 0, 32, 32, 100, 100, 32, 32);

        REQUIRE(batch.getSpriteCount() == 1u);
        REQUIRE(batch.hasSprites());

        batch.end();
        batch.shutdown();
    }
}
