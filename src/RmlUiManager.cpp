#include "SimulacrumEngine.hpp"
#include "RmlUiManager.hpp"
#include <spdlog/spdlog.h>
#include <RmlUi/Core.h>
#include <RmlUi_Backend.h>

namespace Simulacrum
{
  bool RmlUiManager::init(int width, int height)
  {
    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    if (!Rml::Initialise())
    {
      spdlog::critical("Rml::Initialise() failed");
      return false;
    }

    Rml::Context* context = Rml::CreateContext("default", Rml::Vector2i(width, height));

    if (context == nullptr)
    {
      return false;
    }

    return true;
  }

  void RmlUiManager::clean()
  {
    is_shutdown_ = true;
  }
}
