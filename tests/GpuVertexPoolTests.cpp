#include <spdlog/spdlog.h>
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


struct VertexPoolTestFixture : public GPUTestFixture
{
  VertexPoolTestFixture()
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

  ~VertexPoolTestFixture()
  {
    if (device && device->isInitialized())
    {
      device->shutdown();
    }
  }

  Simulacrum::GPUDevice* device = nullptr;
};


TEST_CASE_PERSISTENT_FIXTURE(VertexPoolTestFixture, "Vertex Pool Tests")
{
  SECTION("Begin frame returns mapped pointer")
  {
    REQUIRE(device->isInitialized());

    Simulacrum::GPUVertexPool pool;
    pool.init(device->get(), sizeof(Simulacrum::SpriteVertex));
    REQUIRE(pool.isInitialized());

    void* ptr = pool.beginFrame();

    REQUIRE(ptr != nullptr);
    REQUIRE(pool.getMappedPtr() == ptr);

    pool.endFrame(0);
    pool.shutdown();
  }

  SECTION("End frame records vertex count")
  {
    REQUIRE(device->isInitialized());

    Simulacrum::GPUVertexPool pool;
    pool.init(device->get(), sizeof(Simulacrum::SpriteVertex));
    REQUIRE(pool.isInitialized());

    pool.beginFrame();
    pool.endFrame(100);

    REQUIRE(pool.getVertexCount() == 100u);
    REQUIRE(pool.getMappedPtr() == nullptr);

    pool.shutdown();
  }

  SECTION("Frame cycle advances")
  {
    REQUIRE(device->isInitialized());

    Simulacrum::GPUVertexPool pool;
    pool.init(device->get(), sizeof(Simulacrum::SpriteVertex));
    REQUIRE(pool.isInitialized());

    // Cycle through 3 frames
    for (int frame = 0; frame < 3; ++frame)
    {
      void* ptr = pool.beginFrame();
      REQUIRE(ptr != nullptr);

      // Write some vertices
      Simulacrum::SpriteVertex* vertices = static_cast<Simulacrum::SpriteVertex*>(ptr);
      vertices[0] = Simulacrum::SpriteVertex{ float(frame), 0.0f, 0.0f, 0.0f, 255, 255, 255, 255 };

      pool.endFrame(1);
    }

    // Frame index should wrap around (0, 1, 2, 0, ...)
    // Pool should still be functional
    void* ptr = pool.beginFrame();
    REQUIRE(ptr != nullptr);
    pool.endFrame(0);

    pool.shutdown();
  }

  SECTION("No GPU stall with triple buffering")
  {
    REQUIRE(device->isInitialized());

    Simulacrum::GPUVertexPool pool;
    pool.init(device->get(), sizeof(Simulacrum::SpriteVertex));
    REQUIRE(pool.isInitialized());

    // Simulate multiple frames - triple buffering should prevent stalls
    for (int frame = 0; frame < 10; ++frame)
    {
      void* ptr = pool.beginFrame();
      REQUIRE(ptr != nullptr);

      // Simulate writing vertices
      Simulacrum::SpriteVertex* vertices = static_cast<Simulacrum::SpriteVertex*>(ptr);
      for (size_t i = 0; i < 1000; ++i)
      {
        vertices[i] = Simulacrum::SpriteVertex{
          float(i), float(frame),
          0.0f, 0.0f,
          255, 255, 255, 255
        };
      }

      pool.endFrame(1000);
    }

    pool.shutdown();
  }
}
