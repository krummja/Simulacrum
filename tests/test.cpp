#include <spdlog/spdlog.h>
#include <catch2/catch_test_macros.hpp>

#include "TimestepManager.hpp"


unsigned int Factorial(unsigned int number)
{
    return number <= 1 ? number : Factorial(number - 1) * number;
}

int testTSManager()
{
    Simulacrum::TimestepManager ts = Simulacrum::TimestepManager();

    ts.startFrame();
    ts.endFrame();

    return 1;
}

TEST_CASE("TimestepManager is set up", "[timestep]")
{
    REQUIRE(testTSManager() == 1);
}
