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


TEST_CASE("Composite UBO Size", "[composite_ubo]")
{
  REQUIRE(sizeof(Simulacrum::CompositeUBO) == 32u);
  REQUIRE(offsetof(Simulacrum::CompositeUBO, subPixelOffsetX) == 0u);
  REQUIRE(offsetof(Simulacrum::CompositeUBO, subPixelOffsetY) == 4u);
  REQUIRE(offsetof(Simulacrum::CompositeUBO, zoom) == 8u);
  REQUIRE(offsetof(Simulacrum::CompositeUBO, _pad0) == 12u);
}
