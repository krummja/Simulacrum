#include "engine/core/game_app.hpp"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::trace);
    engine::core::GameApp app;
    app.run();
    return 0;
}
