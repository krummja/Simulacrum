#pragma once

#include <string>
#include <memory>
#include "EventTypeId.hpp"

namespace Simulacrum {

  class Event;

  using EventPtr = std::shared_ptr<Event>;
  using EventWeakPtr = std::weak_ptr<Event>;

  class Event {
  public:
    virtual ~Event() = default;

    virtual void update() = 0;
    virtual void execute() = 0;
    virtual void reset() = 0;
    virtual void clean() = 0;

    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    virtual std::string getTypeName() const = 0;
    virtual std::string getTypeId() const = 0;

    virtual bool isActive() const { return active_; }
    virtual bool setActive(bool active) { active_ = active; }

    virtual int getPriority() const { return priority_; }
    virtual void setPriority(int priority) { priority_ = priority; }

    virtual bool checkConditions() = 0;

    virtual void setUpdateFrequency(int frames_per_update) { update_frequency_ = frames_per_update; }
    virtual int getUpdateFrequency() const { return update_frequency_; }

    virtual void onMessage([[maybe_unused]] const std::string& message) {}

    virtual bool shouldUpdate() const;

    virtual void setCooldown(float seconds) { cooldown_time_ = seconds; }
    virtual float getCooldown() const { return cooldown_time_; }
    virtual bool isOnCooldown() const { return on_cooldown_; }
    virtual void startCooldown();
    virtual void resetCooldown() { on_cooldown_ = false; cooldown_timer = 0.0f; }
    virtual void updateCooldown(float delta_time);

    virtual bool isOneTime() const { return one_time_event_; }
    virtual void setOneTime(bool one_time) { one_time_event_ = one_time; }
    virtual bool hasTriggered() const { return has_triggered_; }

  private:
    bool active_{true};
    int priority_{0};
    int update_frequency_{1};

    bool on_cooldown_{false};
    float cooldown_time_{0.0f};
    float cooldown_timer{0.0f};

    bool one_time_event_{false};
    bool has_triggered_{false};

    mutable int frame_counter_{0};
  };

} // namespace Simulacrum
