// #include "thread_system.hpp"
#include "simulacrum_engine.hpp"
#include "timestep_manager.hpp"
#include "utilitylib/resource_path.hpp"
#include <array>
#include <chrono>
#include <format>
#include <numeric>
#include <string_view>
#include <spdlog/spdlog.h>

constexpr std::string_view APPLICATION_NAME{"Simulacrum"};


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    spdlog::info("Initializing {}", APPLICATION_NAME);

    Simulacrum::ResourcePath::init();

    bool res_exists = Simulacrum::ResourcePath::exists("Debug");
    spdlog::info("Resource directory exist: {}", res_exists);

    // Simulacrum::ThreadSystem& thread_system = Simulacrum::ThreadSystem::Instance();

    Simulacrum::SimulacrumEngine& engine = Simulacrum::SimulacrumEngine::Instance();

    if (!engine.init(APPLICATION_NAME)) {
            spdlog::critical("Init {} failed", APPLICATION_NAME);
            spdlog::info("Cleaning up after initialization failure");
            engine.clean();
        return -1;
    }

    // state manager

    spdlog::info("Starting main loop");

    Simulacrum::TimestepManager ts = Simulacrum::TimestepManager();

    ts.startFrame();
    spdlog::info("Current FPS: {}", ts.getCurrentFPS());
    ts.endFrame();

    // Simulacrum::TimestepManager& ts = engine.getTimestepManager();

    // while (engine.isRunning()) {
    //     ts.startFrame();

    //     engine.handleEvents();

    //     while (ts.shouldUpdate()) {
    //         engine.update(ts.getUpdateDeltaTime());
    //     }

    //     engine.render();

    //     engine.present();

    //     ts.endFrame();
    // }

    // engine.clean();

    return 0;
}
