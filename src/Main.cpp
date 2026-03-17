#include "ThreadSystem.hpp"
#include "TimestepManager.hpp"

#include <spdlog/spdlog.h>

#include "Engine.hpp"
#include "ResourcePath.hpp"

constexpr std::string_view APPLICATION_NAME{ "Simulacrum" };

bool InitializeThreadSystem()
{
  Simulacrum::ThreadSystem& thread_system = Simulacrum::ThreadSystem::Instance();

  try
  {
    if (!thread_system.init())
    {
      spdlog::critical("Failed to initialize thread system");
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


int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[])
{
  spdlog::set_level(spdlog::level::debug);

  // Initialize resource path resolution
  // This must be called BEFORE the thread system is initialized
  Simulacrum::ResourcePath::init();

  // Initialize thread management
  if (!InitializeThreadSystem())
  {
    return -1;
  }

  // Initialize engine
  Simulacrum::Engine& engine = Simulacrum::Engine::Instance();

  if (!engine.init(APPLICATION_NAME))
  {
    spdlog::critical("Init {} failed", APPLICATION_NAME);
    spdlog::info("Cleaning up after initialization failure");
    engine.clean();
    return -1;
  }

  auto ts = Simulacrum::TimestepManager();

  while (Simulacrum::Engine::Instance().isRunning())
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
