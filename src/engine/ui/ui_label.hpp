#ifndef UI_LABEL_HPP_
#define UI_LABEL_HPP_

#include "ui_element.hpp"
#include "../utils/math.hpp"
#include "../render/text_renderer.hpp"
#include <string>
#include <string_view>

namespace engine::ui {

    class UILabel final : public UIElement {
    public:
        UILabel(
            engine::render::TextRenderer& text_renderer,
            std::string_view text,
            std::string_view font_id,
            int font_size = 16,
            engine::utils::FColor text_color = {1.0f, 1.0f, 1.0f, 1.0f},
            glm::vec2 position = {0.0f, 0.0f}
        );

        void render(engine::core::Context& context) override;

        std::string_view getText() const { return text_; }
        std::string_view getFontId() const { return font_id_; }
        int getFontSize() const { return font_size_; }
        const engine::utils::FColor& getTextFColor() const { return text_fcolor_; }

        void setText(std::string_view text);
        void setFontId(std::string_view font_id);
        void setFontSize(int font_size);
        void setTextFColor(engine::utils::FColor text_fcolor);

    private:
        engine::render::TextRenderer& text_renderer_;
        std::string text_;
        std::string font_id_;
        int font_size_;
        engine::utils::FColor text_fcolor_ = {1.0f, 1.0f, 1.0f, 1.0f};

    };

} // namespace engine::ui

#endif // UI_LABEL_HPP_