#include "title_scene.hpp"
#include "../../engine/utils/math.hpp"
#include "../../engine/core/context.hpp"
#include "../../engine/render/renderer.hpp"
#include "../../engine/core/game_state.hpp"
#include "../../engine/resource/resource_manager.hpp"
#include "../../engine/input/input_manager.hpp"
#include "../../engine/ui/ui_manager.hpp"
#include "../../engine/ui/ui_panel.hpp"
#include "../../engine/ui/ui_label.hpp"
#include "../../engine/ui/ui_button.hpp"
#include <spdlog/spdlog.h>

namespace game::scene {

    TitleScene::TitleScene(
        engine::core::Context& context,
        engine::scene::SceneManager& scene_manager
    ): engine::scene::Scene("TitleScene", context, scene_manager) {}

    void TitleScene::init() {
        if (is_initialized_) {
            return;
        }

        createUI();

        Scene::init();
    }

    void TitleScene::update(float delta_time) {
        Scene::update(delta_time);
    }

    void TitleScene::createUI() {
        auto window_size = context_.getGameState().getLogicalSize();

        if (!ui_manager_->init(window_size)) {
            return;
        }

        float button_width = 96.0f;
        float button_height = 32.0f;
        float button_spacing = 20.0f;
        int num_buttons = 4;

        float panel_width = num_buttons * button_width + (num_buttons - 1) * button_spacing;
        float panel_height = button_height;

        float panel_x = (window_size.x - panel_width) / 2.0f;
        float panel_y = window_size.y * 0.65f;

        auto test_panel = std::make_unique<engine::ui::UIPanel>(
            glm::vec2(panel_x, panel_y),
            glm::vec2(panel_width, panel_height)
        );

        test_panel->setBackgroundColor(engine::utils::FColor{0.0, 0.0, 0.0, 1.0});

        glm::vec2 current_button_pos = glm::vec2(0.0f, 0.0f);
        glm::vec2 button_size = glm::vec2(button_width, button_height);

        auto start_button = std::make_unique<engine::ui::UIButton>(
            context_,
            "assets/textures/ui/Start1.png",
            "assets/textures/ui/Start2.png",
            "assets/textures/ui/Start3.png",
            current_button_pos,
            button_size,
            [this]() { this->onStartGameClick(); }
        );

        test_panel->addChild(std::move(start_button));

        spdlog::debug("Added start button and test panel");

        // auto test_label = std::make_unique<engine::ui::UILabel>(
        //     context_.getTextRenderer(),
        //     "Hello, world!",
        //     "assets/fonts/VonwaonBitmap-16px.ttf",
        //     16,
        //     engine::utils::FColor{0.8f, 0.8f, 0.8f, 1.0f}
        // );

        // test_panel->addChild(std::move(test_label));

        ui_manager_->addElement(std::move(test_panel));
    }

    void TitleScene::onStartGameClick() {
        spdlog::debug("Start clicked!");
    }

} // namespace game::scene
