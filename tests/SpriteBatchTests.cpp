#include <spdlog/spdlog.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include "TimestepManager.hpp"
#include "GpuDevice.hpp"
#include "GpuBuffer.hpp"
#include "GpuTexture.hpp"
#include "GpuTransferBuffer.hpp"
#include "GpuVertexPool.hpp"
#include "GpuSampler.hpp"
#include "GpuTypes.hpp"
#include "SpriteBatch.hpp"

#include "GpuTestFixture.hpp"


struct SpriteBatchTestFixture : public GPUTestFixture
{
  SpriteBatchTestFixture()
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

  ~SpriteBatchTestFixture()
  {
    if (device && device->isInitialized())
    {
      device->shutdown();
    }
  }

  Simulacrum::GPUDevice* device = nullptr;
};


TEST_CASE_PERSISTENT_FIXTURE(SpriteBatchTestFixture, "Sprite Batching Tests")
{
  SECTION("Vertex Positions Correct")
  {
    REQUIRE(device->isInitialized());

    Simulacrum::SpriteBatch batch;
    batch.init(device->get());

    std::vector<Simulacrum::SpriteVertex> vertices(Simulacrum::SpriteBatch::MAX_VERTICES);
    batch.begin(vertices.data(), vertices.size(), nullptr, nullptr, 256.0f, 256.0f);

    // Draw sprite at (100, 200) with size (32, 32)
    batch.draw(0, 0, 32, 32, 100, 200, 32, 32);

    batch.end();

    // Verify quad positions (top-left, top-right, bottom-right, bottom-left)
    // Vertex 0: top-left
    REQUIRE(vertices[0].x == Catch::Approx(100.0f).margin(0.001f));
    REQUIRE(vertices[0].y == Catch::Approx(200.0f).margin(0.00f));

    // Vertex 1: top-right
    REQUIRE(vertices[1].x == Catch::Approx(132.0f).margin(0.001f));
    REQUIRE(vertices[1].y == Catch::Approx(200.0f).margin(0.001f));

    // Vertex 2: bottom-right
    REQUIRE(vertices[2].x == Catch::Approx(132.0f).margin(0.001f));
    REQUIRE(vertices[2].y == Catch::Approx(232.0f).margin(0.001f));

    // Vertex 3: botom-left
    REQUIRE(vertices[3].x == Catch::Approx(100.0f).margin(0.001f));
    REQUIRE(vertices[3].y == Catch::Approx(232.0f).margin(0.001f));

    batch.shutdown();
  }
}
