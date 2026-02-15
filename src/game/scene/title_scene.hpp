#ifndef TITLE_SCENE_HPP_
#define TITLE_SCENE_HPP_

#include "../../engine/scene/scene.hpp"
#include <memory>

namespace engine::object {
    class GameObject;
}

namespace engine::scene {
    class SceneManager;
}

namespace game::scene {

    class TitleScene final : public engine::scene::Scene {

    public:
        TitleScene(
            engine::core::Context& context,
            engine::scene::SceneManager& scene_manager
        );

        ~TitleScene() override = default;

        void init() override;
        void update(float delta_time) override;

        TitleScene(const TitleScene&) = delete;
        TitleScene& operator=(const TitleScene&) = delete;
        TitleScene(TitleScene&&) = delete;
        TitleScene& operator=(TitleScene&&) = delete;

    private:
        void createUI();

    };

} // namespace game::scene

#endif // TITLE_SCENE_HPP_