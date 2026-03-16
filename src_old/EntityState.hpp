#pragma once

namespace Simulacrum
{

    class EntityState
    {
    public:
        virtual void enter() = 0;
        virtual void update(float delta_time) = 0;
        virtual void exit() = 0;
        virtual ~EntityState() = default;
    };

} // namespace Simulacrum
