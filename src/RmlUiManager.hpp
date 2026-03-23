#ifndef SIMULACRUM_RML_UI_MANAGER_HPP_
#define SIMULACRUM_RML_UI_MANAGER_HPP_

#include <RmlUi/Core.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace Simulacrum
{
  class RmlUiManager
  {
  public:
    ~RmlUiManager()
    {
      if (!is_shutdown_)
      {
        clean();
      }
    }

    static RmlUiManager& Instance()
    {
      static RmlUiManager instance;
      return instance;
    }

    bool init(int width, int height);

    void update(float delta_time);

    void reset();

    void clean();

    bool isInitialized() const { return is_initialized_; }

    bool isShutdown() const { return is_shutdown_; }

  private:
    Rml::Context* context_;

    bool is_initialized_{ false };
    bool is_shutdown_{ false };

    RmlUiManager(const RmlUiManager&) = delete;
    RmlUiManager& operator=(const RmlUiManager&) = delete;

    RmlUiManager() = default;
  };
}

#endif // SIMULACRUM_RML_UI_MANAGER_HPP_
