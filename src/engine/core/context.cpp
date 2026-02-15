#include "context.hpp"
#include "../input/input_manager.hpp"
#include "../render/renderer.hpp"
#include "../render/text_renderer.hpp"
#include "../resource/resource_manager.hpp"
#include <spdlog/spdlog.h>

namespace engine::core {

    Context::Context(
        engine::input::InputManager& input_manager,
        engine::render::Renderer& renderer,
        engine::render::TextRenderer& text_renderer,
        engine::resource::ResourceManager& resource_manager,
        engine::core::GameState& game_state
    )
        : input_manager_(input_manager)
        , renderer_(renderer)
        , text_renderer_(text_renderer)
        , resource_manager_(resource_manager)
        , game_state_(game_state)
    {
        spdlog::trace("  Bound InputManager to Context.");
        spdlog::trace("  Bound Renderer to Context.");
        spdlog::trace("  Bound TextRenderer to Context.");
        spdlog::trace("  Bound ResourceManager to Context");
        spdlog::trace("  Bound GameState to Context");
    }

} // namespace engine::core
