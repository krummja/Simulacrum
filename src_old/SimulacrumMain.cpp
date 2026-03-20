#include "ThreadSystem.hpp"
#include "TimestepManager.hpp"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"

#include "SimulacrumEngine.hpp"
#include "GpuDevice.hpp"
#include "GpuRenderer.hpp"
#include "ResourcePath.hpp"
#include "ThreadSystem.hpp"

#include <cstdlib>
#include <format>
#include <future>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <chrono>
#include <format>
#include <numeric>
#include <string_view>
#include <spdlog/spdlog.h>

constexpr std::string_view APPLICATION_NAME{ "Simulacrum" };


bool initialize_thread_system()
{
  Simulacrum::ThreadSystem& thread_system = Simulacrum::ThreadSystem::Instance();

  try
  {
    if (!thread_system.init())
    {
      spdlog::critical("Failed to initialized thread system");
      return false;
    }
  }

  catch (const std::exception& exc)
  {
    spdlog::critical("Exception during thread system initialization: {}", exc.what());
    return false;
  }

  return true;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  spdlog::set_level(spdlog::level::debug);

  spdlog::info("Initializing {}", APPLICATION_NAME);

  Simulacrum::ResourcePath::init();

  bool res_exists = Simulacrum::ResourcePath::exists("Debug");
  spdlog::info("Resource directory exist: {}", res_exists);

  if (!initialize_thread_system())
  {
    return -1;
  }

  // Engine

  Simulacrum::SimulacrumEngine& engine = Simulacrum::SimulacrumEngine::Instance();

  if (!engine.init(APPLICATION_NAME))
  {
    spdlog::critical("Init {} failed", APPLICATION_NAME);
    spdlog::info("Cleaning up after initialization failure");
    engine.clean();
    return -1;
  }

  // engine.getStateManager()->pushState("Loading State");

  Simulacrum::TimestepManager ts = Simulacrum::TimestepManager();


  while (engine.isRunning())
  {
    ts.startFrame();

    engine.handleEvents();

    while (ts.shouldUpdate())
    {
      float dt = ts.getUpdateDeltaTime();
      engine.update(dt);
    }

    engine.render();

    engine.present();

    ts.endFrame();
  }

  engine.clean();

  return 0;
}
