#ifndef SHADER_MANAGER_HPP_
#define SHADER_MANAGER_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>

namespace engine::resource {

    class ShaderManager final {
        friend class ResourceManager;

    public:
      explicit ShaderManager();

      ShaderManager(const ShaderManager&) = delete;
      ShaderManager& operator=(const ShaderManager&) = delete;
      ShaderManager(ShaderManager&&) = delete;
      ShaderManager& operator=(ShaderManager&&) = delete;

    private:

    };

} // namespace engine::resource

#endif // SHADER_MANAGER_HPP_