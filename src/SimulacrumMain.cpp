#include "thread_system.hpp"
#include "simulacrum_engine.hpp"
#include "timestep_manager.hpp"
#include "resource_path.hpp"

#include <array>
#include <chrono>
#include <format>
#include <numeric>
#include <string_view>
#include <spdlog/spdlog.h>

constexpr std::string_view APPLICATION_NAME{ "Simulacrum" };


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  spdlog::set_level(spdlog::level::debug);

  spdlog::info("Initializing {}", APPLICATION_NAME);

  Simulacrum::ResourcePath::init();

  bool res_exists = Simulacrum::ResourcePath::exists("Debug");
  spdlog::info("Resource directory exist: {}", res_exists);

  Simulacrum::ThreadSystem& thread_system = Simulacrum::ThreadSystem::Instance();

  try
  {
    if (!thread_system.init())
    {
      spdlog::critical("Failed to initialized thread system");
      return -1;
    }
  }
  catch (const std::exception& exc)
  {
    spdlog::critical("Exception during thread system initialization: {}", exc.what());
    return -1;
  }

  Simulacrum::SimulacrumEngine& engine = Simulacrum::SimulacrumEngine::Instance();

  if (!engine.init(APPLICATION_NAME))
  {
    spdlog::critical("Init {} failed", APPLICATION_NAME);
    spdlog::info("Cleaning up after initialization failure");
    engine.clean();
    return -1;
  }

  engine.getStateManager()->pushState("Loading State");

  Simulacrum::TimestepManager ts = Simulacrum::TimestepManager();

  ts.startFrame();
  spdlog::info("Current FPS: {}", ts.getCurrentFPS());
  ts.endFrame();

  spdlog::info("Starting main loop");

  while (engine.isRunning())
  {
    ts.startFrame();

    engine.handleEvents();

    while (ts.shouldUpdate())
    {

      engine.update(ts.getUpdateDeltaTime());

    }

    engine.render();

    engine.present();

    ts.endFrame();
  }

  engine.clean();

  return 0;
}
