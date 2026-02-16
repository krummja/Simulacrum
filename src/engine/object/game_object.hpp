#ifndef GAME_OBJECT_HPP_
#define GAME_OBJECT_HPP_

#include "component.hpp"
#include <string_view>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <utility>
#include <spdlog/spdlog.h>

namespace engine::core {
    class Context;
}

namespace engine::object {

    class GameObject final {
    public:
        GameObject(std::string_view name = "", std::string_view tag = "");

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = delete;
        GameObject& operator=(GameObject&&) = delete;

        void setName(std::string_view name) {
            name_ = name;
        }

        void setTag(std::string_view tag) {
            tag_ = tag;
        }

        void setNeedRemove(bool need_remove) {
            need_remove_ = need_remove;
        }

        std::string_view getName() const {
            return name_;
        }

        std::string_view getTag() const {
            return tag_;
        }

        bool isNeedRemove() const {
            return need_remove_;
        }

        void update(float delta_time, engine::core::Context& context);
        void render(engine::core::Context& context);
        void clean();
        void handleInput(engine::core::Context& comtext);

        template <typename T>
        bool hasComponent() const {
            static_assert(std::is_base_of<engine::object::Component, T>::value, "T must inherit Component");
            return components_.contains(std::type_index(typeid(T)));
        }

        template <typename T>
        T* getComponent() const {
            static_assert(std::is_base_of<engine::object::Component, T>::value, "T must inherit Component");
            auto type_index = std::type_index(typeid(T));
            auto it = components_.find(type_index);
            if (it != components_.end()) {
                return static_cast<T*>(it->second.get());
            }
            return nullptr;
        }

        template <typename T, typename... Args>
        T* addComponent(Args&&... args) {
            static_assert(std::is_base_of<engine::object::Component, T>::value, "T must inherit Component");
            auto type_index = std::type_index(typeid(T));
            if (hasComponent<T>()) {
                return getComponent<T>();
            }

            auto new_component = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = new_component.get();
            new_component->setOwner(this);
            components_[type_index] = std::move(new_component);
            ptr->init();
            return ptr;
        }

        template <typename T>
        void removeComponent() {
            static_assert(std::is_base_of<engine::object::Component, T>::value, "T must inherit Component");
            auto type_index = std::type_index(typeid(T));
            auto it = components_.find(type_index);
            if (it != components_.end()) {
                it->second->clean();
                components_.erase(it);
            }
        }

    private:
        std::string name_;
        std::string tag_;
        std::unordered_map<std::type_index, std::unique_ptr<engine::object::Component>> components_;
        bool need_remove_ = false;

    };

} // namespace engine::object

#endif // GAME_OBJECT_HPP_
