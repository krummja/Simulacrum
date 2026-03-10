#include "GpuShaderManager.hpp"
#include "ResourcePath.hpp"
#include <format>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <SDL3_shadercross/SDL_shadercross.h>

namespace Simulacrum
{

  GPUShaderManager& GPUShaderManager::Instance()
  {
    static GPUShaderManager instance;
    return instance;
  }

  bool GPUShaderManager::init(SDL_GPUDevice* device)
  {
    if (!device)
    {
      spdlog::error("GPUShaderManager::init: null device");
      return false;
    }

    device_ = device;

    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device_);

    if (formats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
      spdlog::info("GPUShaderManager: using SPIR-V shaders");
    }
    else
    {
      spdlog::error("GPUShaderManager: no supported shader format");
      return false;
    }

    return true;
  }

  void GPUShaderManager::shutdown()
  {
    size_t count = shaders_.size();
    for (auto& [name, shader] : shaders_)
    {
      if (shader && device_)
      {
        SDL_ReleaseGPUShader(device_, shader);
      }
    }

    shaders_.clear();
    device_ = nullptr;

    spdlog::info("GPUShaderManager::shutdown: released {} shaders", count);
  }

  SDL_GPUShader* GPUShaderManager::loadShader(
    const std::string& base_path,
    SDL_GPUShaderStage stage,
    const ShaderInfo& info
  )
  {
    if (!device_)
    {
      spdlog::error("GPUShaderManager::loadShader: not initialized");
      return nullptr;
    }

    // Check cache using base path as key
    auto it = shaders_.find(base_path);
    if (it != shaders_.end())
    {
      return it->second;
    }

    SDL_GPUShader* shader = nullptr;

    // Build full path with extension, then resolve
    // ResourcePath::resolve() checks if file exists, so we must include the extension
    std::string path = Simulacrum::ResourcePath::resolve(base_path + ".spv");
    shader = loadSPIRV(path, stage, info);

    if (shader)
    {
      shaders_[base_path] = shader;
      spdlog::debug("Loaded shader: {}", base_path);
    }

    return shader;
  }

  SDL_GPUShader* GPUShaderManager::getShader(const std::string& name) const
  {
    auto it = shaders_.find(name);
    return (it != shaders_.end()) ? it->second : nullptr;
  }

  bool GPUShaderManager::hasShader(const std::string& name) const
  {
    return shaders_.find(name) != shaders_.end();
  }

  SDL_GPUShader* GPUShaderManager::loadSPIRV(
    const std::string& path,
    SDL_GPUShaderStage stage,
    const ShaderInfo& info
  )
  {
    // Read binary file
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
      spdlog::error("Failed to open shader file: {}", path);
      return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
      spdlog::error("Failed to read shader file: {}", path);
      return nullptr;
    }

    SDL_GPUShaderCreateInfo create_info{};
    create_info.code = buffer.data();
    create_info.code_size = buffer.size();
    create_info.entrypoint = "main";
    create_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    create_info.stage = stage;
    create_info.num_samplers = info.num_samplers;
    create_info.num_storage_textures = info.num_storage_textures;
    create_info.num_storage_buffers = info.num_storage_buffers;
    create_info.num_uniform_buffers = info.num_uniform_buffers;

    SDL_GPUShader* shader = SDL_CreateGPUShader(device_, &create_info);

    if (!shader)
    {
      spdlog::error("Failed to create SPIR-V shader {}: {}", path, SDL_GetError());
    }

    return shader;
  }

} // namespace Simulacrum
