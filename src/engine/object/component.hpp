#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

namespace engine::core {
    class Context;
}

namespace engine::object {
    class GameObject;

    class Component {
        friend class engine::object::GameObject;

    public:
        Component() = default;
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;

        void setOwner(engine::object::GameObject* owner) {
            owner_ = owner;
        }

        engine::object::GameObject* getOwner() const {
            return owner_;
        }

    protected:
        engine::object::GameObject* owner_ = nullptr;

        virtual void init() {}
        virtual void handleInput(engine::core::Context&) {}
        virtual void update(float, engine::core::Context&) = 0;
        virtual void render(engine::core::Context&) {}
        virtual void clean() {}

    };

} // namespace engine::object

#endif // COMPONENT_HPP_
