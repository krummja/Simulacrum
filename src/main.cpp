#include "engine/core/game_app.hpp"
#include "engine/scene/scene_manager.hpp"
#include "game/scene/title_scene.hpp"
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void setupInitialScene(engine::scene::SceneManager& scene_manager) {
    auto title_scene = std::make_unique<game::scene::TitleScene>(
        scene_manager.getContext(),
        scene_manager
    );

    scene_manager.requestPushScene(std::move(title_scene));
}


int test_process() {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return -1;
    }

    static SDL_Window* window = NULL;
    static SDL_Renderer* renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("examples/renderer", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        return -1;
    }

    SDL_SetRenderLogicalPresentation(renderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    bool should_quit_ = false;

    while (!should_quit_) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    should_quit_ = true;
                    break;
                default:
                    break;
            }
        }

        const double now = ((double)SDL_GetTicks()) / 1000.0;
        const float red = (float) (0.5 + 0.5 * SDL_sin(now));
        const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
        const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
        SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    return 0;

}


int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::trace);

    engine::core::GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}
